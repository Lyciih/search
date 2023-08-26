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

	webgpu_init();
}



function websocket_connect_function()
{
	socket = new WebSocket("wss://lyciih.idv.tw:1223/");
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
		if(event.data.charAt(0) == "c")
		{
			websocket_output.value = websocket_output.value + '\n' + event.data.substring(1);
			websocket_output.scrollTop = websocket_output.scrollHeight;
		}
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
	socket.close(1000, "正常斷開");
	console.log("close");
}

function websocket_send_function()
{
	var message = "c" + websocket_input.value;
	socket.send(message);
	websocket_input.value = "";
}


async function webgpu_init()
{
	if (!navigator.gpu)
	{
		console.log("webGPU is not supported");
	}
	else
	{
		console.log("webGPU is supported");
	}


	const adapter = await navigator.gpu.requestAdapter();
	if(!adapter)
	{
		throw Error("Couldn't request webgpu adapter.");
	}
}
