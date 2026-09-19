#!/usr/bin/env bash
SESSION="afl"

# Start a new detached session with the primary window
tmux new-session -d -s "$SESSION" -n primary
tmux send-keys -t "$SESSION":primary \
  'afl-fuzz -M primary -i - -o state -g 4000 -m none -- syslogd-asan/syslogd -P 1234' C-m

# Create 23 secondary windows
for i in $(seq -w 01 22); do
  tmux new-window -t "$SESSION" -n secondary"$i"
  tmux send-keys -t "$SESSION":secondary"$i" \
    "afl-fuzz -S secondary${i} -i - -o state -g 4000 -m none -- syslogd-normal/syslogd -P 1234" C-m
done

# Attach to session
tmux attach -t "$SESSION"

