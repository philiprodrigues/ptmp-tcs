#!/bin/bash

# this is a really dirty script that will leave behind zombies

ptmpdir="$HOME/dev/ptmp"
ptmptcsdir="$HOME/dev/ptmp-tcs"
datadir="/data/fast/bviren/ptmp-dumps/2019-05-03"

recv=$ptmpdir/build/test/check_recv 
fsource=$ptmptcsdir/build/test/check_fixtimes
replay=$ptmpdir/build/test/check_replay
window=$ptmpdir/build/test/check_window
sorted=$ptmpdir/build/test/check_sorted
tcfinder=$ptmptcsdir/build/test/check_tcfinder

for n in {1..8}
do
    $fsource -E 10000 \
             ifile -f $datadir/FELIX_BR_50$n.dump \
             osock -p PUSH -a bind -e tcp://127.0.0.1:666$n > log.files.$n 2>&1 &
    
    $replay -s1.0 -c15 \
            input -p PULL -a connect -e tcp://127.0.0.1:666$n \
            output -p PUSH -a bind -e tcp://127.0.0.1:777$n > log.replay.$n 2>&1 &

    $window -c 3 -s 3000 -b 150000 \
            input -p PULL -a connect -e tcp://127.0.0.1:777$n \
            output -p PUSH -a connect -e tcp://127.0.0.1:9990 > log.window.$n 2>&1 &


done

$sorted -c 10  \
        input -p PULL -a bind -e tcp://127.0.0.1:9990 \
        output -p PUSH -a bind -e tcp://127.0.0.1:9991 > log.sorted 2>&1 &

$tcfinder input -p PULL -a connect -e tcp://127.0.0.1:9991 \
          output -p PUSH -a bind -e tcp://127.0.0.1:9992 > log.tcfinder 2>&1 &


$recv -v5 -p PULL -a connect -e tcp://127.0.0.1:9992




