#! /bin/bash
work_SESSION="search"



if tmux has-session -t work_SESSION 2>/dev/null; then
	echo "session 已存在"
else

	tmux new-session -d -s $work_SESSION
	tmux rename-window -t $work_SESSION:0 'work'
	tmux send-keys -t $work_SESSION:0 'cd ..&&clear' C-m 

	tmux split-window -h -p 50 -t $work_SESSION:0
	tmux send-keys -t $work_SESSION:0 'cd ..&&clear' C-m 
fi
tmux attach -t $work_SESSION:work



#tmux split-window -v -p 50 -t $SESSION:0
#tmux select-pane -t $SESSION:0 -L
#tmux split-window -v -p 50 -t $SESSION:0

#開啟同步指令並連線測試
#tmux set-option synchronize-panes on
#tmux send-keys -t $SESSION:0 'make test' C-m 
#tmux set-option synchronize-panes off



