#! /bin/bash
monitor_SESSION="monitor"


if tmux has-session -t $monitor_SESSION 2>/dev/null; then
	echo "session 已經存在"
else
	tmux new-session -d -s $monitor_SESSION
	tmux rename-window -t $monitor_SESSION:0 'monitor'
	tmux split-window -h -p 50 -t $monitor_SESSION:0
	#tmux set-option synchronize-panes on
	tmux send-keys -t $monitor_SESSION:0 'cd ..' C-m 
	tmux send-keys -t $monitor_SESSION:0 'clear' C-m 
	#sleep 1
	#tmux set-option synchronize-panes off
	tmux select-pane -t $monitor_SESSION:0 -L
	tmux send-keys -t $monitor_SESSION:0 'cd ..' C-m 
	tmux send-keys -t $monitor_SESSION:0 'clear' C-m 
	tmux send-keys -t $monitor_SESSION:0 'top' C-m 

	tmux select-pane -t $monitor_SESSION:0 -R
	tmux send-keys -t $monitor_SESSION:0 'sudo tcpdump port 1223' C-m 

	tmux split-window -v -p 90 -t $monitor_SESSION:0
	tmux send-keys -t $monitor_SESSION:0 'cd ..' C-m 
	tmux send-keys -t $monitor_SESSION:0 'clear' C-m 
fi
tmux attach -t $monitor_SESSION:monitor

