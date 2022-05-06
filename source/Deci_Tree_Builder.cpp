#include "Deci_Tree_Builder.h"
#include "Deci_Tree_Node.h"

//keeps track of threadCount
const int threadCount(std::thread::hardware_concurrency());

LeafPtrPair Deci_Tree_Builder::Expand_Leaf_(std::shared_ptr<Leaf_Node> inputLeafPtr, std::mutex& cv_m)
{
    try{

        //create Tree_Node

        TreeNodePtr newTreeNode( new Deci_Tree_Node(inputLeafPtr->nodeData_, inputLeafPtr->depth_, inputLeafPtr->parent_, inputLeafPtr->label_));
        newTreeNode->Train_();

        //this could be seen as "dirty" design wise
        //We are downcasting the parent pointer into a derived child pointer
        LeafPtrPair retValue(std::dynamic_pointer_cast<Leaf_Node>(newTreeNode->leftChild_), std::dynamic_pointer_cast<Leaf_Node>(newTreeNode->rightChild_));
        
        //if parent isnt NULL (current node isnt root)
        if(inputLeafPtr->parent_ != NULL)
        {
            //find parent
            Tree_Node* leafParent (inputLeafPtr->parent_);

            //check if the inputLeaf is the left or right child of its parent
            //and update the child
            std::unique_lock<std::mutex> resLock (cv_m);
            if(inputLeafPtr == leafParent->leftChild_)
            {
                leafParent->leftChild_.reset();
                leafParent->leftChild_ = newTreeNode;
            }else
            {
                leafParent->rightChild_.reset();
                leafParent->rightChild_ = newTreeNode;
            }
            resLock.unlock();
        }

            

        return retValue;

    }catch(std::logic_error &e)
    {

        std::cout << "abandoning leaf split, cannot split space" << std::endl;
        return LeafPtrPair(LeafPtr(),LeafPtr());

    }
}

void Deci_Tree_Builder::MakeTree_(const Data_List& inputDataList)
{

    //NOTE: PRIORITY QUEUE NOT USED
    //REASON: makes no difference since all leafs are expanded anyway
    //initialize empty PRIORITY queue of pointers to Leaf_Node pointers
    //As in, access to the parent's pointer to children
    //all nodes will have an entropy measure, and this priority queue is sorted by entropy
    //created root node, and queued first potential splits into priority queue
    std::stack<LeafPtr> leafPrioQueue;

    //pointer to a root node
    //Create a new root node as a leaf, and give it the external constraints as parent constraints and the data 
    LeafPtr tempRoot (new Leaf_Node(inputDataList, 1, NULL, -1));
    //create treeNode
    //if entropy isnt 0
    if(tempRoot->Get_Entropy() > entropyCutoff_ and tempRoot->depth_ < maxdepth_)
    {
        TreeNodePtr temp = std::make_shared<Deci_Tree_Node>(tempRoot->nodeData_, tempRoot->depth_, tempRoot->parent_, tempRoot->label_);
        temp->Train_();
        treeRoot_ = temp;

        //Uses dynamic casting, to push root onto prio queue
        leafPrioQueue.push(std::dynamic_pointer_cast<Leaf_Node>(std::dynamic_pointer_cast<Tree_Node>(treeRoot_)->leftChild_));
        leafPrioQueue.push(std::dynamic_pointer_cast<Leaf_Node>(std::dynamic_pointer_cast<Tree_Node>(treeRoot_)->rightChild_));
    }else
    {
        if(tempRoot->Get_Entropy() <= entropyCutoff_)
        {
            tempRoot->splittingCandidate_ = false;
        }
        treeRoot_ = tempRoot;
    }
    


    if(!threadCount){ throw std::runtime_error("no threads can be created");}

    //fixed size schedule array for threads
    //first member holds created thread, second holds the state
    //0 not running, 1 running, 2 finished running
    std::vector<std::pair<std::thread, int>> schedule(threadCount); 
    //initialize thread state
    for(int i = 0; i < threadCount; i++)
    {
        schedule[i].second = 0;
    }

    // Each node tracks current tree depth
    //loop while Pqueue isnt empty
    //ANY_THREADS_RUNNING: implemented with a vector, like a "clock" for an employee
    //CAN_RUN = !leafPrioQueue.empty() && ANY_THREADS_avail
    //DONE = leafPrioQueue.empty() && !ANY_THREAD_RUNNING
    bool done = 0;
    //condition variable used to check if atleast one thread is free
    std::condition_variable cv;
    //RISKY: multiple mutexes can end in deadlock
    //mutex used with condition variable, unlocked with cv.wait()
    std::mutex cv_m;
    while(!done)
    {
        //forced this thread to wait unless it can run, or the loop is done
        //when a thread calls notify, it means it is immediatly free and has cleaned everything up, so it is open to be "joined"
        //the conditional on cv: does it needs to be in a func? yes, while waiting we want to check the status of threads
        std::unique_lock<std::mutex> cv_lk(cv_m);
        while(!(Cond_(leafPrioQueue, schedule,0) || Cond_(leafPrioQueue, schedule,1)))
        {
            cv.wait(cv_lk);
        }

        //This MUST be mutex locked here, since threads will place leaves in the queue
        //
        //for every thread, if a thread isnt running: run it on an avail leaf
        //                  if a thread is done, join it and run on an avail leaf
        for(int i = 0; i < threadCount; i++)
        {
            if(schedule[i].second != 1)
            {
                if(schedule[i].second == 2){ 
                    schedule[i].first.join(); 
                    schedule[i].second = 0;
                }

                if(!leafPrioQueue.empty())
                {
                    LeafPtr popedLeaf = leafPrioQueue.top();
                    leafPrioQueue.pop();

                    //check entropy here
                    //we also check here for depth
                    if(popedLeaf->Get_Entropy() > entropyCutoff_ and popedLeaf->depth_ < maxdepth_)
                    {
                        schedule[i].first = std::thread(&Deci_Tree_Builder::_leafThread_, this, popedLeaf, std::ref(leafPrioQueue), std::ref(schedule), i, std::ref(cv_m), std::ref(cv));
                        schedule[i].second = 1;
                    }else if(popedLeaf->Get_Entropy() <= entropyCutoff_)
                    {
                        popedLeaf->splittingCandidate_ = false;
                    }
                }
            }
        }

        if(Cond_(leafPrioQueue, schedule, 0))
        {
            done = 1;
        }
        cv_lk.unlock();

    }
}

std::string Deci_Tree_Builder::GetTitle() const
{
    return std::string("trepan");
}


//  //print a slew of output data for each depth of the tree (to save time)
//  void Deci_Tree_Builder::Get_All(bool classBalance)
//  {
//      std::ofstream outFStream;

//      //Make new file
//      outFStream.open("trepan_output_testing.txt");
//      //first attach the validation of the upper model
//      outFStream << filename_ << " upper model acc: " << oraclePtr_->Validate()*100 << "%" << std::endl;
//      for(size_t d = 1; d <= maxdepth_; d++)
//      {
//          if(!classBalance)
//          {
//              outFStream << filename_ << " depth: " << d <<" Unbalanced Model Parity: " <<  Model_Parity(d, classBalance) << "." << " Leaf Count: " << treeRoot_->CountLeaf(d) << std::endl;
//          }else
//          {
//              outFStream << filename_ << " depth: " << d <<" Class balanced Model Parity: " <<  Model_Parity(d, classBalance) << "." << " Leaf Count: " << treeRoot_->CountLeaf(d) << std::endl;
//          }
//      }
//      outFStream.close();

//      //make file for constraints
//      for(size_t d = 1; d <= maxdepth_; d++)
//      {
//          PrintLeafConstr(d);
//      }

//  }

//  void Deci_Tree_Builder::PrintLeafConstr(size_t depth)
//  {
//      ConstrVecPtr constrMatVecPtr = treeRoot_->PrintConstr(depth);

//      //append to file
//      std::ofstream outFStream;
//      outFStream.open("trepan_Leaf_Constraints.txt");
//      //first attach the validation of the upper model
//      outFStream << filename_ << " leaf constraint list" << std::endl;

//      std::vector<arma::mat>::const_iterator vecIter = constrMatVecPtr->begin();
//      for(;vecIter != constrMatVecPtr->end(); vecIter++)
//      {
//          outFStream << "real depth: " << vecIter->n_rows << std::endl;
//          
//          outFStream << "query depth: " << depth << std::endl;

//          for(int i = 0; i < vecIter->n_rows; i++)
//          {
//              for(int j = 0; j < (vecIter->n_cols - 1); j++)
//              {
//                  outFStream << (*vecIter)(i,j) << ',';
//              }
//              outFStream << (*vecIter)(i,vecIter->n_cols - 1);
//              outFStream << std::endl;
//          }
//          
//          outFStream << std::endl;
//      }
//      outFStream.close();
//      
//  }
