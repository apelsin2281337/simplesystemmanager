#!/bin/bash
sudo -v
sudo -E nohup SystemManager >/dev/null 2>&1 &
exit 0
