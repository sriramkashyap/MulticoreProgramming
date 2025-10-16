#!/bin/bash

echo Started: `date`

trace_file="trace_file.perfetto-trace"

epoch=`date +%s`
mv ${trace_file} trace_${epoch}.trace

# Start Perfetto tracer
sudo ~/perfetto/tracebox -o "$trace_file" --txt -c scheduling.cfg -d

# Wait a while for it to start (we can do better)
sleep 1

# Run test program
$1

# Wait for trace file
while [ ! -f "$trace_file" ]; do
  sleep 1
done


sudo chmod 666 ${trace_file}

echo Completed: `date`
