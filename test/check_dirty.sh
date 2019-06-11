#!/bin/bash

# this is a really dirty script that will leave behind zombies

ptmpdir="$HOME/dev/ptmp"
ptmptcsdir="$HOME/dev/ptmp-tcs"
#datadir="/data/fast/bviren/ptmp-dumps/2019-05-03"
database="/data/fast/bviren/ptmp-dumps/2019-06-10/FELIX_BR_"

#fsource=$ptmptcsdir/build/test/check_fixtimes
fsource=czmqat
replay=$ptmpdir/build/test/check_replay
window=$ptmpdir/build/test/check_window
sorted=$ptmpdir/build/test/check_sorted
tcfinder=$ptmptcsdir/build/test/check_tcfinder
recv=$ptmpdir/build/test/check_recv 

# proxies run for some time and then destroy
jobtime=200

# how long to wait for slow streams, in ms.
tardy_ms=10000

for n in {601..610}
do
    $fsource -E 10000 \
             ifile -f ${database}$n.dump \
             osock -p PUSH -a bind -e tcp://127.0.0.1:666$n > log.files.$n 2>&1 &
    
    # 666n->777n
    $window -c $jobtime -s 3000 -b 150000 \
            input -p PULL -a connect -e tcp://127.0.0.1:666$n \
            output -p PUSH -a connect -e tcp://127.0.0.1:777$n > log.window.$n 2>&1 &

    # 777n->9990
    $replay -c $jobtime -s 50.0 \
            input -p PULL -a bind -e tcp://127.0.0.1:777$n \
            output -p PUSH -a connect -e tcp://127.0.0.1:9990 > log.replay.$n 2>&1 &


done

$sorted -t $tardy_ms -c $jobtime  \
        input  -p PULL -a bind -e tcp://127.0.0.1:9990 \
        output -p PUSH -a bind -e tcp://127.0.0.1:9991 > log.sorted 2>&1 &

$tcfinder -c $jobtime  input -p PULL -a connect -e tcp://127.0.0.1:9991 \
          output -p PUSH -a bind -e tcp://127.0.0.1:9992 > log.tcfinder 2>&1 &


$recv -v5 -p PULL -a connect -e tcp://127.0.0.1:9992




