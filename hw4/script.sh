#!/bin/bash
echo "Test Script Starts"

echo "Running ./randtrack 1 50> rt1.out"
./randtrack 1 50 > rt1.out

echo "Running ./randtrack_global_lock 1 50> rt21.out"
./randtrack_global_lock 1 50 > rt2.out

echo "Running ./randtrack_global_lock 2 50> rt22.out"
./randtrack_global_lock 2 50> rt22.out

echo "Running ./randtrack_global_lock 4 50> rt23.out"
./randtrack_global_lock 4 50> rt23.out



