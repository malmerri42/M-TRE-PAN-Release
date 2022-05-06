
xline(3.57839693972576)
yline(4.5698677768743297)
xline(-3.2320377959400099)
yline(-4.7421819003062904)
refline(3.9303784211838587e-06/2.0765316074187269e-05,1.5258895936841405e-05/2.0765316074187269e-05)
refline(-8.5186786530419517e-06/-3.2714219114089827e-05,4.0484092153105026e-05/-3.2714219114089827e-05)
refline(-2.4974489527811464e-05/-2.8474551391615752e-05,-9.2780164728199122e-06/-2.8474551391615752e-05)
hold on
plot(3.57839693972576,-0.30570433289802196,'r*')
plot(3.57839693972576,1.412131168454539,'r*')
plot(3.57839693972576,3.4643795441637657,'r*')
plot(3.57839693972576,1.412131168454539,'r*')
plot(1.2385352269401055,1.412131168454539,'r*')
plot(3.57839693972576,-0.30570433289802196,'r*')
plot(3.57839693972576,3.4643795441637657,'r*')
plot(-0.72004715814265918,-0.30570433289802196,'r*')
%prev inner hcube dims:
xline(3.57839693972576, '--r')
yline(1.412131168454539,'--r')
xline(3.57839693972576, '--r')
yline(-0.30570433289802196,'--r')
%prev prev inner hcube dims:
xline(3.57839693972576, '--g')
yline(1.412131168454539,'--g')
xline(-3.2320377959400099, '--g')
yline(-4.7421819003062904,'--g')
%Diagnosis:
%it seems as though intersections are getting dropped, loosen up the
%"tightness" of intersections, allow 10X freedom (or more)
%

%CAUTION: we had this idea before: it failed because you would be checking
%choose Dim - 1 combinations, eg: 3 dimensions, but there might be a way to
%use the previous intersections.

%design upgrades: It  may cost more computation time, but keeping a list of
%intersections and upgrading that, might be a better approach, since we
%will just make the hypercube surround it when generating data. 
%A new constraint will be checked against all previous constraints one by
%one,