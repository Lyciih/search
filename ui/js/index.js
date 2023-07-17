var socket;
var websocket;
var websocket_send;
var websocket_input;
var websocket_output;

window.onload = function()
{
	websocket = document.getElementById("websocket");
	websocket.addEventListener("click", websocket_connect_function);
	
	websocket_send = document.getElementById("websocket_send");
	websocket_send.addEventListener("click", websocket_send_function);

	websocket_input = document.getElementById("websocket_input");
	websocket_output = document.getElementById("websocket_output");
}



function websocket_connect_function()
{
	socket = new WebSocket("ws://59.127.115.95:1223/");
	socket.onopen = function()
	{
		console.log("connect");
		websocket.textContent = "斷線";
		websocket.removeEventListener("click", websocket_connect_function);
		websocket.addEventListener("click", websocket_close_function);
	};

	socket.onmessage = function(event)
	{
		console.log(event.data);
		websocket_output.value = websocket_output.value + '\n' + event.data;
		websocket_output.scrollTop = websocket_output.scrollHeight;
	};


	socket.onclose = function(event)
	{
		console.log("disconnect");
		websocket.textContent = "連線";
		websocket.removeEventListener("click", websocket_close_function);
		websocket.addEventListener("click", websocket_connect_function);
	}
}



function websocket_close_function()
{
	socket.close(3000, "abc");
	console.log("close");
}

function websocket_send_function()
{
	var message = websocket_input.value;
	socket.send(message);
	websocket_input.value = "";
}
