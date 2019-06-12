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

# speed of replay.  It gives how many hardware ticks to replay per
# microsecond.  Ie, 50.0 means normal speed.  I can be artificially
# slowed or sped up but tardy_ms may require change
hwtickperusec=50.0

tokill=""
tpsorted_endpoints=""
for fn in 601 603 605 607 609
do
    oport=9$fn
    $fsource -E 10000 \
             ifile -f ${database}${fn}.dump \
             osock -p PUSH -a bind -e tcp://127.0.0.1:$oport > log.files.$fn 2>&1 &
    tokill="$tokill $!"

    iport=$oport
    oport=8$fn
    $window -c $jobtime -s 3000 -b 150000 \
            input -p PULL -a connect -e tcp://127.0.0.1:$iport \
            output -p PUSH -a connect -e tcp://127.0.0.1:$oport > log.window.$fn 2>&1 &
    tokill="$tokill $!"

    iport=$oport
    oport=7$fn
    tpsorted_endpoints="-e tcp://127.0.0.1:$oport $tpsorted_endpoints"
    $replay -c $jobtime -s $hwtickperusec \
            input -p PULL -a bind -e tcp://127.0.0.1:$iport \
            output -p PUSH -a connect -e tcp://127.0.0.1:$oport > log.replay.$fn 2>&1 &
    tokill="$tokill $!"

done

oport=6666
$sorted -t $tardy_ms -c $jobtime  \
        input  -p PULL -a bind $tpsorted_endpoints \
        output -p PUSH -a bind -e tcp://127.0.0.1:$oport > log.sorted 2>&1 &
tokill="$tokill $!"
iport=$oport
oport=5555
$tcfinder -c $jobtime  input -p PULL -a connect -e tcp://127.0.0.1:$iport \
          output -p PUSH -a bind -e tcp://127.0.0.1:$oport > log.tcfinder 2>&1 &
tokill="$tokill $!"

iport=$oport
$recv -v5 -p PULL -a connect -e tcp://127.0.0.1:$iport

kill -9 $tokill



