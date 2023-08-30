var screen_width;

var socket;
var websocket;
var websocket_send;
var websocket_input;
var websocket_output;


window.onload = function()
{
	//ui -----------------------------------------------------------------------------------
	screen_width = document.getElementById("screen_width_html");
	screen_width.innerText = "寬度:" + window.innerWidth;
	

	


	

	
	//websocket -----------------------------------------------------------------------------
	websocket = document.getElementById("websocket_html");
	websocket.addEventListener("click", websocket_connect_function);
	
	websocket_send = document.getElementById("websocket_send_html");
	websocket_send.addEventListener("click", websocket_send_function);

	websocket_input = document.getElementById("websocket_input_html");
	websocket_output = document.getElementById("websocket_output_html");

	//webgpu -----------------------------------------------------------------------------------
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

	const device = await adapter.requestDevice();
	if(!device)
	{
		throw Error("Couldn't request webgpu device.");
	}

	console.log(adapter.limits);

	const main_canvas = document.getElementById("main_canvas_html");
	const main_context = main_canvas.getContext("webgpu");
	const main_format = navigator.gpu.getPreferredCanvasFormat();

	main_context.configure({
		device, 
		format: main_format,
	});

	const main_module = device.createShaderModule({
		label: 'red triangle',
		code:`
		@vertex fn vs(@builtin(vertex_index) vertexIndex : u32) -> @builtin(position) vec4f {
			var pos = array<vec2f, 3>(
				vec2f(0.0, 0.5),
				vec2f(-0.5, -0.5),
				vec2f(0.5, -0.5)
			);

			return vec4f(pos[vertexIndex], 0.0, 1.0);
		}

		@fragment fn fs() -> @location(0) vec4f {
			return vec4f(1, 0, 0, 1);
		}
		`,
	});


	const pipeline = device.createRenderPipeline({
		label: 'red triangle pipeline',
		layout: 'auto',
		vertex: {
			module: main_module,
			entryPoint: 'vs',
		},
		fragment: {
			module: main_module,
			entryPoint: 'fs',
			targets: [{ format: main_format }],
		},
		primitive: {
			topology: 'line-strip',
		},
	});

	const renderPassDescriptor = {
		label: 'canvas renderPass',
		colorAttachments: [
			{
				clearValue: [0.3, 0.3, 0.3, 1],
				loadOp: 'clear',
				storeOp: 'store',
			},
		],
	};


	function render() {
		//為了在每次canvas改變大小時能重新取得繪製尺寸，在這裡才取得尺寸，而不是在 renderPassDescriptor 中先定義
		renderPassDescriptor.colorAttachments[0].view = main_context.getCurrentTexture().createView();

		const encoder = device.createCommandEncoder({label: 'encoder'});

		const pass = encoder.beginRenderPass(renderPassDescriptor);
		pass.setPipeline(pipeline);
		pass.draw(3);
		pass.end();

		const commandBuffer = encoder.finish();
		device.queue.submit([commandBuffer]);
	}


	function screen_resize()
	{
		console.log(window.innerWidth);
		main_canvas.width = main_canvas.offsetWidth - 2;
		main_canvas.height = main_canvas.offsetHeight - 2;
		screen_width.innerText = "寬度:" + window.innerWidth;
		render();
	}
	
	addEventListener("resize", screen_resize);

	main_canvas.width = main_canvas.offsetWidth - 2;
	main_canvas.height = main_canvas.offsetHeight - 2;
	render();


}
