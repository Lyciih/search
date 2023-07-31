#! /bin/bash
SESSION="server_test"

#開出四格
tmux new-session -d -s $SESSION
tmux split-window -h -p 50 -t $SESSION:0
tmux split-window -v -p 50 -t $SESSION:0
tmux select-pane -t $SESSION:0 -L
tmux split-window -v -p 50 -t $SESSION:0

#開啟同步指令並連線測試
tmux set-option synchronize-panes on
tmux send-keys -t $SESSION:0 'make test' C-m 
#tmux set-option synchronize-panes off

tmux attach -t server_test

