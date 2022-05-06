#include "Tree_Builder.h"
#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iomanip>
#include <iostream>

typedef std::shared_ptr<Leaf_Node> LeafPtr;

//keeps track of threadCount
const int threadCount(std::thread::hardware_concurrency());
//implementation level condition helper to check if leafQueue is empty and no more threads are running
//, passing int == 0 checks if done, int == 1 checks if canRun (see below for both)
bool _Cond(std::stack<LeafPtr>& leafPrioQueue, std::pair<std::thread,int>* schedule, int cond);

Tree_Builder::Tree_Builder(const char* file, std::shared_ptr<Oracle> inputOracle , bool relabelFlag, unsigned long maxdepth, double entropyCutoff):
    treeRoot_(NULL),
    oraclePtr_(inputOracle),
    inputData_(file,relabelFlag,inputOracle),
    filename_(file)

{
    //if maxdepth is 0, then it is unbounded
    std::numeric_limits<unsigned long> uLongLim;

    maxdepth_ = (maxdepth == 0) ? uLongLim.max() : maxdepth;

    //if entropyCutoff is 0, then it is min of a double
    std::numeric_limits<double> doubleLim;
    entropyCutoff_ = (entropyCutoff == 0)  ? doubleLim.min() : entropyCutoff;



}
    
BaseNodePtr Tree_Builder::generate(void)
{

    MakeTree_(inputData_);

    return treeRoot_;
    
}

bool Tree_Builder::nodeCompare_( BaseNodePtr A, BaseNodePtr B)
{
    if( A->Get_Entropy() > B->Get_Entropy())
    {
        return 1;
    }
    else
    {
        return 0;
    }

}

LeafPtrPair Tree_Builder::Expand_Leaf_(std::shared_ptr<Leaf_Node> inputLeafPtr, std::mutex& cv_m)
{
    try{

        //create Tree_Node

        TreeNodePtr newTreeNode( new Tree_Node(inputLeafPtr->nodeData_, inputLeafPtr->depth_, inputLeafPtr->parent_, inputLeafPtr->label_));
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


//implementation level condition helper to check if leafQueue is empty and no more threads are running
//, passing int == 0 checks if done, int == 1 checks if can_distribute
    //done = leafPrioQueue.empty() && !ANY_THREAD_RUNNING
    //can_distribute = !leafPrioQueue.empty() && ANY_THREADS_avail
bool Tree_Builder::Cond_(std::stack<LeafPtr>& leafPrioQueue, std::vector<std::pair<std::thread,int>>& schedule, int cond)
{

    bool anyThreadRunning = 0;
    bool anyThreadAvail = 0;
    //check if any threads are running and avail
    //start with checking if schedule has any running threads
    for(int i = 0; i < threadCount; i++)
    {
        if(schedule[i].second == 1)
        {
            anyThreadRunning = 1;
        
        }else
        {
            anyThreadAvail = 1;
        }
    }

    bool empty = 0;
    //check PrioQueue
    if(leafPrioQueue.empty())
    {
        empty = 1;
    }

    if(cond == 0)
    {

        if(empty && !anyThreadRunning)
        {
            //we are done, no more threads left and prioQueue empty
            return 1;
        }else
        {
            return 0;
        }

    }else
    {
        if(!empty && anyThreadAvail)
        {
            return 1;
        }else
        {
            return 0;
        }
    }

}

//implementation level helper
void Tree_Builder::_leafThread_(LeafPtr popedLeaf, std::stack<LeafPtr>& leafPrioQueue, std::vector<std::pair<std::thread,int>>& schedule, const int threadID,  std::mutex& cv_m, std::condition_variable& cv)
{

    //The leaf is checked if it can be split, if not it remains whole and the algorithm moves onto the next leaf
    LeafPtrPair leafChildren = Expand_Leaf_(popedLeaf, cv_m);
        

    std::unique_lock<std::mutex>cvLk(cv_m);
    //check for null
    if(leafChildren.first)
    {
        PushLeafNodes_(leafPrioQueue, leafChildren);
    }
    //finished, so "clock out" before notifiy
    schedule[threadID].second = 2;
    cvLk.unlock();
    cv.notify_all();
    

     
}

void Tree_Builder::PushLeafNodes_(std::stack<LeafPtr>& leafPrioQueue, LeafPtrPair& ptrPair)
{

    leafPrioQueue.push(ptrPair.first);
    leafPrioQueue.push(ptrPair.second);
}

void Tree_Builder::PushRootLeafNodes_(std::stack<LeafPtr>& leafPrioQueue, BaseNodePtr& rootPtr)
{

    //Uses dynamic casting, to push root onto prio queue
    leafPrioQueue.push(std::dynamic_pointer_cast<Leaf_Node>(std::dynamic_pointer_cast<Tree_Node>(treeRoot_)->leftChild_));
    leafPrioQueue.push(std::dynamic_pointer_cast<Leaf_Node>(std::dynamic_pointer_cast<Tree_Node>(treeRoot_)->rightChild_));
}

void Tree_Builder::MakeTree_(const Data_List& inputDataList)
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
        TreeNodePtr temp = std::make_shared<Tree_Node>(tempRoot->nodeData_, tempRoot->depth_, tempRoot->parent_, tempRoot->label_);
        temp->Train_();
        treeRoot_ = temp;

        PushRootLeafNodes_(leafPrioQueue, treeRoot_);
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
                if(schedule[i].second == 2)
                { 
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
                        schedule[i].first = std::thread(&Tree_Builder::_leafThread_, this, popedLeaf, std::ref(leafPrioQueue), std::ref(schedule), i, std::ref(cv_m), std::ref(cv));
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

std::tuple<double, double, double, double, double> Tree_Builder::ConfusionMat_(size_t depthLim, const DataVec& genDataVec, int (Base_Tree_Node::*NodeEval)(const arma::Row<double>&, size_t) const ) const
{

    //to count evaluated predicitons
    double truePositive = 0;
    double trueNegative = 0;
    double falsePositive = 0;
    double falseNegative = 0;
    double uncertain = 0;

    //for all the validation data compare against model predictions
    DataVec::const_iterator constIter = genDataVec.begin();
    for(; constIter != genDataVec.end(); constIter++)
    {
        //DONT account for eval returning -1 on "negative" predictions 
        //since tree will return -1 on negative predictions
        //derefrenced pointer turning it into instance of obj and then called passed member derefrenced func 
        if( oraclePtr_->Eval(constIter->first) == ((*treeRoot_).*NodeEval)(constIter->first, depthLim) )
        {
            if(((*treeRoot_).*NodeEval)(constIter->first, depthLim) == -1)
            {
                trueNegative++;

            }else if(((*treeRoot_).*NodeEval)(constIter->first, depthLim) == 1)
            {
                truePositive++;
            }else
            {
                throw;
            }

        }else 
        {

            if(((*treeRoot_).*NodeEval)(constIter->first, depthLim) == -1)
            {
                falseNegative++;

            }else if(((*treeRoot_).*NodeEval)(constIter->first, depthLim) == 1)
            {
                falsePositive++;

            }else if(((*treeRoot_).*NodeEval)(constIter->first, depthLim) == 0)
            {
                uncertain++;
            }
            else
            {
                throw;
            }
        }

    }

    return std::tuple<double, double, double, double, double>(trueNegative, truePositive, falseNegative, falsePositive, uncertain);
}

std::tuple<double, double, double, double, double> Tree_Builder::Boundry_Model_Parity(size_t depthLim, double percent, double margin) 
{
    

    std::shared_ptr<DataVec> dataVecPtr = oraclePtr_->MarginFilter(treeRoot_->Sample_DataVec_As_Leaf(percent),margin);
    

    return ConfusionMat_(depthLim, *dataVecPtr, &Base_Tree_Node::Certain_Evaluate); 

}

std::tuple<double, double, double, double, double> Tree_Builder::Model_Parity(size_t depthLim, double percent)
{
    

    const DataVec &genDataVec = treeRoot_->Sample_DataVec_As_Leaf(percent);

    return ConfusionMat_(depthLim, genDataVec, &Base_Tree_Node::Evaluate); 

}

std::tuple<double, double, double, double, double> Tree_Builder::Certain_Model_Parity(size_t depthLim, double percent)
{
    

    const DataVec &genDataVec = treeRoot_->Sample_DataVec_As_Leaf(percent);

    return ConfusionMat_(depthLim, genDataVec, &Base_Tree_Node::Certain_Evaluate); 

}

//over whole tree
std::tuple<double, double, double, double, double> Tree_Builder::Model_Parity(double percent)
{
    return Model_Parity(maxdepth_, percent);
}

double Tree_Builder::Validate(const char* filename)
{
    arma::mat allData;
    allData.load(filename);

    arma::mat data = allData.cols(0,allData.n_cols - 2);
    arma::Col<double> labels = allData.col(allData.n_cols - 1);

    double correctPrediction = 0;
    double totalPrediction = 0;
    
    //over all data
    for(size_t i = 0; i < data.n_rows; i++)
    {
        //account for Evaluate returning -1 on negagtive instead of 0
        double evalTemp = treeRoot_->Evaluate(data.row(i));
        if(evalTemp == -1){ evalTemp = 0;}

        if( labels(i) == evalTemp )
        {
            correctPrediction++;
        }

        totalPrediction++;
    }

    return (correctPrediction/totalPrediction)* 100;

}

double Tree_Builder::Train_Acc()
{
    double correctPrediction = 0;
    double totalPrediction = 0;
    //over all data
    DataVec data = inputData_.Get_Data();
    for(size_t i = 0; i < data.size(); i++)
    {
        double evalTemp = treeRoot_->Evaluate(data[i].first);

        if( data[i].second == evalTemp )
        {
            correctPrediction++;
        }

        totalPrediction++;
    }

    return (correctPrediction/totalPrediction)* 100;
}

double Tree_Builder::Norm_Train_Acc()
{
    double correctPrediction = 0;
    double totalPrediction = 0;
    //over all data
    DataVec data = inputData_.Get_Data();
    double posDataAmnt = 0;
    double negDataAmnt = 0;

    //find class weights
    double rollingSum = 0;
    double negWeight = 1;
    double posWeight = 1;
    
    //sum over all labels {-1 , 1} to get amount of positive and negative labels that are "extra"
    for(size_t i = 0; i < data.size(); i++)
    {
        rollingSum += data[i].second;

        if(data[i].second == 1)
        {
            posDataAmnt++;
        }else
        {
            negDataAmnt++;
        }
    }

    if(rollingSum < 0)
    {
        rollingSum = -1 * rollingSum;
        posWeight += rollingSum/posDataAmnt;
    }else
    {
        negWeight += rollingSum/negDataAmnt;
    }

    for(size_t i = 0; i < data.size(); i++)
    {
        double evalTemp = treeRoot_->Evaluate(data[i].first);

        if(data[i].second == -1)
        {
            if( data[i].second == evalTemp )
            {
                correctPrediction += 1 + negWeight;
            }

            totalPrediction += 1 + negWeight;
        }else
        {

            if( data[i].second == evalTemp )
            {
                correctPrediction += 1 + posWeight;
            }

            totalPrediction += 1 + posWeight;
        }
    }

    return (correctPrediction/totalPrediction)* 100;
}

double Tree_Builder::MissAmnt()
{
    double incorrectPrediction = 0;
    double totalPrediction = 0;
    //over all data
    DataVec data = inputData_.Get_Data();
    for(size_t i = 0; i < data.size(); i++)
    {
        double evalTemp = treeRoot_->Evaluate(data[i].first);

        if( data[i].second != evalTemp )
        {
            incorrectPrediction++;
        }

        totalPrediction++;
    }

    return (incorrectPrediction);
}

//return a string obj of this title
std::string Tree_Builder::GetTitle() const
{
    return std::string("mtrepan");
}

//return file name without trailing directory info
std::string Tree_Builder::GetFileName() const
{
    std::string rawFileName(filename_);
    std::string slicedFileName = rawFileName.substr(rawFileName.find_last_of('/') + 1);
    return slicedFileName.substr(0,slicedFileName.find_last_of('.'));

}

//print a slew of output data for each depth of the tree (to save time)
void Tree_Builder::Get_All(double percentValid, double margin)
{
    std::ofstream outFStream;

    //Make new file
    outFStream.open(GetTitle() + "_" + GetFileName() + "_" + "output_testing.txt");
    //first attach the validation of the upper model
    outFStream << filename_ << " upper model acc: " << oraclePtr_->Validate()*100 << "%" << std::endl;
    outFStream << "depth, " <<"Model Parity: True Negative, True Positive, False Negative, False Positive, " << "Model_Parity, "<<  "Certain Model Parity, "<< "Boundry Model Parity, " << "Certain Leaf Count, " <<"Leaf Count, " << std::endl;

    for(size_t d = 1; d <= maxdepth_; d++)
    {

        double trueNegative = 0, truePositive = 0, falseNegative = 0, falsePositive = 0, uncertain = 0;
        std::tie(trueNegative, truePositive, falseNegative, falsePositive, uncertain) = Model_Parity(d, percentValid);

        double certTrueNegative = 0, certTruePositive = 0, certFalseNegative = 0, certFalsePositive = 0, CertUncertain = 0;
        std::tie(certTrueNegative, certTruePositive, certFalseNegative, certFalsePositive, CertUncertain) = Certain_Model_Parity(d, percentValid);

        double boundTrueNegative = 0, boundTruePositive = 0, boundFalseNegative = 0, boundFalsePositive = 0, boundUncertain = 0;
        std::tie(boundTrueNegative, boundTruePositive, boundFalseNegative, boundFalsePositive, boundUncertain) = Boundry_Model_Parity(d, percentValid, margin);

        std::vector<double> values{//treeRoot_->Average_Tree_Accuracy(d, percentValid) * 100, treeRoot_->BalancedModelParity(d, percentValid) * 100};
                                    ((truePositive + trueNegative)/(falseNegative + falsePositive + uncertain + truePositive + trueNegative) )* 100, ((certTruePositive + certTrueNegative)/(certFalseNegative + certFalsePositive + CertUncertain + certTruePositive + certTrueNegative) )* 100,
((boundTruePositive + boundTrueNegative)/(boundFalseNegative + boundFalsePositive + boundTruePositive + boundTrueNegative + boundUncertain) )* 100
    };

        outFStream <<  std::setw(7) << d << ',';

        outFStream << std::setw(7) << (size_t) trueNegative << "," << std::setw(7) << (size_t) truePositive << "," << std::setw(7) << (size_t) falseNegative << "," << std::setw(7) << (size_t) falsePositive << ",";

        for(size_t i = 0; i < values.size(); i++){  

            outFStream <<  std::setw(7) << std::fixed << std::setprecision(2) << values[i]  << ",";
        }

        outFStream << std::setw(7) << treeRoot_->CountCertainLeafs(d);
        outFStream << std::setw(7) << treeRoot_->CountLeaf(d) << std::endl;

    }
    outFStream.close();

    //make file for constraints
    for(size_t d = 1; d <= maxdepth_; d++)
    {
        PrintLeafConstr(d);
    }

}

//repurpose this for "global explanation"
//Print as though depth is the depth of the tree
void Tree_Builder::PrintLeafConstr(size_t depth)
{
    Base_Tree_Node::ConstrVecPtr constrMatVecPtr = treeRoot_->PrintConstr(depth);

    //append to file
    std::ofstream outFStream;
    outFStream.open(GetTitle() + "_" + GetFileName()+ "_" + "depth_" + std::to_string(depth) + "_" +"Leaf_Constraints.txt");
    //outFStream << filename_ << std::endl;
    outFStream << "id[1], label[1], constraint[n], bias[1]" << std::endl;

    std::vector<std::tuple<Constraint, int, const Base_Tree_Node*>>::const_iterator vecIter = constrMatVecPtr->begin();

    //Iterate through every node, and give an ID
    unsigned long id = 0;
    for(;vecIter != constrMatVecPtr->end(); vecIter++)
    {
        //this is not printing depth
        //outFStream << "real depth: " << std::get<0>(*vecIter).GetMat() << std::endl;
        
        //outFStream << "query depth: " << depth << std::endl;

        id++;
        //for every row
        for(int i = 0; i < std::get<0>(*vecIter).GetMat().n_rows; i++)
        {
            outFStream << id << ',';
            outFStream << std::get<1>(*vecIter) << ',';
            //for every column
            for(int j = 0; j < (std::get<0>(*vecIter).GetMat().n_cols - 1); j++)
            {
                outFStream << std::get<0>(*vecIter).GetMat()(i,j) << ',';
            }
            outFStream << std::get<0>(*vecIter).GetMat()(i,std::get<0>(*vecIter).GetMat().n_cols - 1);
            outFStream << std::endl;
        }
        
    }

    outFStream.close();
    
}


arma::mat Tree_Builder::FindInter_(const Constraint& input, const Base_Tree_Node* currNode) 
{
    //conduct a depth first search, where when a constraint is accepted to find intersections, it is removed and the constraint that produced an intersection of it is selected next
    //If not intersection is found within the original unpruned constraints, the back up is to fall back to the hcube
    //fails if more than 2d
    if( input.GetMat().n_cols != 3 ){throw;}

    //create some storage for the found intersections
    arma::mat interSecMat(0,2);

    //first copy the input into a temp mat 
    arma::mat realConstrMat = currNode->nodeData_.Get_Constraint().GetMat();

    //initalize the currently considered constraint and the "first" constraint, to be used to find the last intersection
    //arma::Row<double> firstConstraint = realConstrMat.row(0); 
    //arma::Row<double> currConstraint = firstConstraint;
    
    //find the next constr
    //While currentConstr has been changed and isnt validly intersecting with the first constraint keep going
    bool done = false;

    //create stack to keep track of state during dfs search of connecting constraints
    //first matrix stores the remaining constraints, 
    //the size_t maintains our index for the current constraint matrix
    //the first arma::Row is the current constraint used to test against the remaining constraints
    //the second arma::Row is the first constraint, if (after the size of the stack after the initial pop is two or more) the newly selected current constraint prior to being placed on the stack with the updated constraints is also intersecting the first constraint, then we place its valid intersection with the first constraint at the end of the intersection mat and we RETURN it , we are finished
    //the second matrix maintains an ordered matrix of inersections (our final output)
    typedef std::tuple<arma::mat, long, arma::Row<double>, arma::Row<double>, arma::mat> StackTuple ;
    std::stack<StackTuple> stateStack;
    stateStack.emplace(realConstrMat, -1, arma::Row<double>(), arma::Row<double>(), interSecMat); 

    while(!done)
    {
        //pop the top of the stack to continue from that state
        StackTuple currentState = stateStack.top();
        stateStack.pop();

        //populate state parameters
        arma::mat currConstrMat = std::get<0>(currentState);
        long currIdx = std::get<1>(currentState);
        arma::Row<double> currConstr = std::get<2>(currentState);
        arma::Row<double> firstConstr = std::get<3>(currentState);
        arma::mat currIntersecMat = std::get<4>(currentState);

        //if the stack is empty that means that the first constr needs to be updated, along with incrementing the index and shrinking the current constr mat
        if(stateStack.empty())
        {
            long updatedIdx = currIdx + 1; 
            //if the updatedIdx is greater than to the number of available indexable spaces, it is a throw situation
            if(updatedIdx > currConstrMat.n_rows - 1){throw;}
            stateStack.emplace(currConstrMat, updatedIdx, currConstr, firstConstr, currIntersecMat);

            arma::Row<double> updatedFirstConstr = currConstrMat.row(updatedIdx);
            //take out the constr and set it as first constr 
            arma::mat updatedCurrConstrMat = currConstrMat;
            updatedCurrConstrMat.shed_row(updatedIdx);
            //the next state to "work on"
            //this state is distinct in that it has currConstr as now the FirstConstr, this means that while in this state, the constraints being tested need only intersect with the first
            //-constraint, and when one does, the current state is updated, and the next state is initialized with the new currConstr and updated constrMat and the intersection is attached
            stateStack.emplace(updatedCurrConstrMat, 0, updatedFirstConstr, updatedFirstConstr, currIntersecMat);
        }else
        {

            //flag to detect if one valid intersection has been found
            //to detect if we hit a dead-end or if we can now "do stuff"
            bool intersecFound = false;
            //index of found intersection
            size_t foundIntersecIdx = 0;
            //the current intersection under consideration
            arma::Col<double> intersec;
            //the actual found intersection
            arma::Col<double> foundIntersec;
            //continue the for-loop from where that state paused
            for(size_t k = currIdx; k < currConstrMat.n_rows; k++)
            {
                //search for one constraint that intersects currConstr
                if(!intersecFound)
                {
                    arma::mat tempMat = arma::join_vert(currConstrMat.row(k),currConstr);

                    //it does intersect
                    if( arma::solve(intersec, tempMat.cols(0,1),tempMat.col(2), arma::solve_opts::no_approx) )
                    {
                        //the intersection is valid as in, within the constraints
                        if(currNode->nodeData_.Get_Constraint_Ref().Apply_Constraint(intersec.t()))
                        {
                            //track the index and set the intersecFound flag to true
                            intersecFound = true;
                            foundIntersecIdx = k;
                            foundIntersec = intersec;


                        }
                    }
                }
            }

            if(intersecFound)
            {

                //if an intersection was found then store the current state with the updated index of the found constraint index
                stateStack.emplace(currConstrMat, foundIntersecIdx + 1, currConstr, firstConstr, currIntersecMat);
                
                //prepare the next state with the shrunk currConstrMat, a reset index, an updated currConstr and updated currIntersecMat
                arma::mat updatedCurrConstrMat = currConstrMat;
                updatedCurrConstrMat.shed_row(foundIntersecIdx);

                arma::mat updatedCurrIntersecMat = arma::join_vert(currIntersecMat,foundIntersec.t());

                arma::Row<double> updatedCurrConstr = currConstrMat.row(foundIntersecIdx);

                stateStack.emplace(updatedCurrConstrMat, 0, updatedCurrConstr, firstConstr, updatedCurrIntersecMat);

                //dont bother checking if we only have one intersection so far
                if(updatedCurrIntersecMat.n_rows > 1)
                {
                    //check if the found constraint also intersects the firstConstr
                    arma::Col<double> intersec;
                    arma::mat tempMat = arma::join_vert(currConstrMat.row(foundIntersecIdx),firstConstr);
                    if( arma::solve(intersec, tempMat.cols(0,1),tempMat.col(2), arma::solve_opts::no_approx) )
                    {
                        //the intersection is valid as in, within the constraints
                        if(currNode->nodeData_.Get_Constraint_Ref().Apply_Constraint(intersec.t()))
                        {
                            //we are done
                            done = 1;
                            return arma::join_vert(updatedCurrIntersecMat,intersec.t());

                        }
                    }
                }
            }//otherwise it will loop and self pop
        }
    }


    return interSecMat;
}

void Tree_Builder::IntersecList(size_t depthLim) const
{
    //array of 6 arma::mat, one for each coordinate (x,y), and a coordinate pair of matricies for each label
    //each column starts with the number of elements in the column
    arma::mat matArr[3][2] = {
        {arma::mat(), arma::mat()}, 
        {arma::mat(), arma::mat()}, 
        {arma::mat(), arma::mat()}
    };

    std::string stringArr[3] = {"negative","uncertain", "positive"};
    std::string cordArr[2] = {"x", "y"};

    Base_Tree_Node::ConstrVecPtr constrMatVecPtr = treeRoot_->PrintConstr(depthLim);
    
    //for every leaf a matrix of constraints is returned
    //each matrix from FindInter_() represents the vertices of a polygon, each polygon's vertices is placed in its own column in the respective matricies, preceeded by the number of elements in the column
    std::vector<std::tuple<Constraint, int, const Base_Tree_Node*>>::const_iterator constIter = constrMatVecPtr->begin();
    for(; constIter != constrMatVecPtr->end(); constIter++)
    {
        //the matrix of intersections
        arma::mat interSecList = FindInter_(std::get<0>(*constIter), std::get<2>(*constIter));

        for(size_t cord = 0;  cord < (sizeof cordArr / sizeof *cordArr); cord++)
        {
            //seperate into x and y, find size and append to top row
            arma::Col<double> cVals = arma::join_vert(arma::Col<double>{(double)interSecList.n_rows},interSecList.col(cord));
            //append horizontally into appropriate matrix, but first resize the smaller (in rows) matrix
            if(matArr[std::get<1>(*constIter) + 1][cord].n_rows > cVals.n_rows)
            {
                cVals.resize(matArr[std::get<1>(*constIter) + 1][cord].n_rows);

            }else if(matArr[std::get<1>(*constIter) + 1][cord].n_rows < cVals.n_rows)
            {
                matArr[std::get<1>(*constIter) + 1][cord].resize(cVals.n_rows,matArr[std::get<1>(*constIter) + 1][cord].n_cols);
            }

            matArr[std::get<1>(*constIter) + 1][cord] = arma::join_horiz(matArr[std::get<1>(*constIter) + 1][cord], cVals);
        }
    }
     
    for(size_t i = 0; i < (sizeof stringArr / sizeof *stringArr); i++)
    {
        for(size_t j = 0; j < (sizeof cordArr / sizeof *cordArr); j++)
        {
            matArr[i][j].save( "../build/intersectionData/" + cordArr[j]+ "_" + stringArr[i] +  "_intersections_" + GetTitle() + "_" + GetFileName() + "_" + std::to_string(depthLim) + ".csv", arma::csv_ascii);
        }
    }
}

void Tree_Builder::GetAllIntersec() const
{
    for(size_t d = 1; d <= maxdepth_; d++)
    {
        IntersecList(d);
    }
}

bool Tree_Builder::ValidIntersec_(const arma::Row<double>& intersec, const  Constraint& realConstr, const Constraint& hcubeConstr)
{
    if(realConstr.Apply_Constraint(intersec) || hcubeConstr.Apply_Constraint(intersec))
    {
        return true;
    }else
    {
        return false;
    }
}

//depth limit just truncates the generated streams
//not using depth lim right now
void Tree_Builder::LocalExplanation(arma::Mat<double> pointMat)//, size_t depthLim)
{
    //for every point in the matrix
    std::vector<arma::mat> matVec; 
    for(size_t j = 0; j < pointMat.n_rows; j++)
    {
        Local_Exp_Visitor myVisitor(pointMat.row(j));
        treeRoot_->AcceptVisitor(pointMat.row(j),&myVisitor);

        //output entire matrix to csv 
        arma::mat currMat = myVisitor.Get_Mat();

        std::string expFilename(GetTitle() + "_local_exp_point_" + GetFileName()+"_" + std::to_string(j) + ".csv");

        //bad:
        //currMat.save( "temp.txt", arma::csv_ascii);
        //append a header at the top
        //std::ifstream inFStream;
        //inFStream.open("temp.txt");

        std::ofstream outFStream;
        outFStream.open(expFilename);
        outFStream << "For point: <"; 
        for(size_t p = 0; p < pointMat.n_cols; p++)
        {
            outFStream << pointMat(j,p);
            //print a comma, except at the end
            if(p < pointMat.n_cols - 1)
            {
                outFStream << ',';
            }else
            {
                outFStream << ">" << std::endl;
            }
        }

        // prepare the header
        for(size_t z = 0; z < pointMat.n_cols; z++)
        {
            outFStream << "constraint_" << z << ", ";
        }
        outFStream << "bias, prior_entropy, post_entropy, sibling_post_entropy, change_in_entropy, change_in_sibling_entropy, relative_size, sibling_relative_size, weighted_average_post_entropy, label, distance_from_point" << std::endl;

        for(size_t k = 0; k < currMat.n_rows; k++)
        {
            for(size_t m = 0; m < currMat.n_cols; m++)
            {
                outFStream << currMat(k,m);
                
                //print a comma, except at the end
                if(m < currMat.n_cols - 1)
                {
                    outFStream << ',';
                }else
                {
                    outFStream << std::endl;
                }
            }
        }
        outFStream.close();
    }
}

void Tree_Builder::Example_LocExp(size_t num)
{
    //Using the first n examples of the validation data, generate some example local explanations
    arma::mat valDataSub = oraclePtr_->GetVData();

    if(valDataSub.n_rows > num)
    {
        valDataSub = (oraclePtr_->GetVData()).rows(0,num-1);
    }

    LocalExplanation(valDataSub);
}
