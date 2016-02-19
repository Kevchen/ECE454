#!/bin/sh

rm -rf *.out*
rm -rf *.outs*
echo "run randtrack"
/usr/bin/time ./randtrack 4 100 > 1.out
echo "run global_lock"
/usr/bin/time ./randtrack_global_lock 4 100 > 2.out
echo "run tm"
/usr/bin/time ./randtrack_tm 4 100 > 3.out
echo "run list_lock"
/usr/bin/time ./randtrack_list_lock 4 100 >4.out
echo "run element_lock"
/usr/bin/time ./randtrack_element_lock 4 100 >5.out
echo "run reduction"
/usr/bin/time ./randtrack_reduction 4 100 >6.out

#./randtrack 4 0 > 1.out
#./randtrack_global_lock 4 0 > 2.out
#./randtrack_tm 4 0 > 3.out
#./randtrack_list_lock 4 0 >4.out
#./randtrack_element_lock 4 0 >5.out
#./randtrack_reduction 4 0 >6.out

sort -n 1.out > 1.outs
sort -n 2.out > 2.outs
sort -n 3.out > 3.outs
sort -n 4.out > 4.outs
sort -n 5.out > 5.outs
sort -n 6.out > 6.outs

echo "diff global_lock"
diff 1.outs  2.outs
echo "diff tm"
diff 1.outs  3.outs
echo "diff list_lock"
diff 1.outs  4.outs
echo "diff element_lock"
diff 1.outs  5.outs
echo "diff reduction"
diff 1.outs  6.outs
