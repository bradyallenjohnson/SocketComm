#!/bin/sh

echo "exampleTcpServerMain stats will be dumped to the console where it is running"
kill -s USR1 `pgrep exampleTcpServerMain`
