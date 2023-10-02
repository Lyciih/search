var page_size = 64;


class client_data {
	constructor(name = "null", ID = -1) {
		this.name = name;
		this.ID = ID;
		this.polyline = new Float32Array(6);

		this.polyline[0] = 0.0;
		this.polyline[1] = 0.5;
		this.polyline[2] = -0.5;
		this.polyline[3] = -0.5;
		this.polyline[4] = 0.5;
		this.polyline[5] = -0.5;
	}
}


class storage_copy_buffer_value_pair{
	constructor(my_size){
		this.buffer = device.createBuffer({
			label: 'storage copy',
			size: my_size,
			usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
		});
		this.value = new Int32Array(my_size/4); 
	}

	submit(offset = 0){
		device.queue.writeBuffer(this.buffer, offset, this.value);
	}
}

class line_tensor{
	constructor(){
		this.space = page_size / 8;
		this.manage = new storage_copy_buffer_value_pair(page_size / 2);
		this.begin = new storage_copy_buffer_value_pair(page_size);
		this.end = new storage_copy_buffer_value_pair(page_size);
		this.bindGroup = device.createBindGroup({
			label: `line tensor bindGroup`,
			layout: pipeline.getBindGroupLayout(0),
			entries: [
				{ binding: 0, resource: { buffer: canvas_size_unitformBuffer }},
				{ binding: 1, resource: { buffer: this.begin.buffer }},
				{ binding: 2, resource: { buffer: this.end.buffer }},
			],
		});
	}
}



class geometry_data{
	constructor(){
		this.line = [];
		this.line.push(new line_tensor);
	}
}


//變數宣告
var ID_table;
var ID;

var chat_history = {
	value: [],
	count: 0,
};

var command_history = {
	value: [],
	count: 0,
};

var max_client = 1024;
var all_client_data = new Array(max_client);
var all_geometry_data;

for(let i = 0; i < max_client; i++)
{
	all_client_data[i] = new client_data('null', i);
	//console.log(all_client_data[i].ID);
}

var temp = [];
var tempValues;
var device;
var adapter;
var temp_storageBuffer;
var pipeline;
var canvas_size_unitformBuffer;
//元素接口
var screen_width;

var socket;
var websocket_connection;
var chat_send;
var chat_input;
var chat_output;

var command_send;
var command_input;
var command_output;

var main_canvas;
var mode_display;

var main_canvas_rect;
var main_canvas_mouse_location;

//函數接口
var render;


//命令狀態機
var line_state = 0;

const rand = (min, max) => {
	if(min === undefined)
	{
		min = 0;
		max = 1;
	}
	else if(max === undefined)
	{
		max = min;
		min = 0;
	}
	return min + Math.random() * (max - min);
}


//鍵盤
var input_mode = 'normal';

document.onkeydown = function(event) {
	if(event.code == 'Escape'){
		console.log('normal mode');
		document.activeElement.blur();
		input_mode = 'normal';
		mode_display.innerText = "模式: normal";
	}
	else if(input_mode == 'normal'){
		//s鍵
		if(event.code == "KeyS"){
			event.preventDefault();
			console.log('chat mode');
			input_mode = 'chat'
			mode_display.innerText = "模式: chat";
			chat_input.focus();
		}
		//l鍵
		else if(event.code == "KeyL"){
			console.log('line mode');
			input_mode = 'line'
			mode_display.innerText = "模式: line";
			
			message_output_update(command_output, command_history, 'line');
			message_output_update(command_output, command_history, '請輸入第一個點:');
		}
	}
	else if(input_mode == 'chat'){
		//此處用event.key是因為中文注音輸入法按enter與發送衝突的原因
		if(event.key == "Enter"){
			event.preventDefault();
			chat_send_function();
		}
	}
}

function message_output_update(target_interface, target, string)
{
	target_interface.scrollTop = target_interface.scrollHeight;
	if(target.count < 500)
	{
		target.value.push(string);
		target.count++;
	}
	else
	{
		target.value.shift();
		target.value.push(string);
	}
	target_interface.value = '';
	for(let i = 0; i < target.count; i++)
	{
		console.log(target.count);
		target_interface.value = target_interface.value + target.value[i] + '\n';
	}
}



//滑鼠
//萬一在輸入模式中用滑鼠按了其他地方，模式也要跟著變
document.onclick = function(event) {
	console.log(event.target.id);
	if(event.target.id == ''){
		input_mode = 'normal';
		mode_display.innerText = "模式: normal";
		console.log('normal mode');
	}
	
	if(event.target.id == 'chat_input_html'){
		input_mode = 'chat';
		mode_display.innerText = "模式: chat";
		console.log('chat mode');
	}
	
	if(event.target.id == 'main_canvas_html'){
		if(input_mode == 'line'){
			if(line_state == 0)
			{
				temp[0] = event.pageX - main_canvas_rect.left;
				temp[1] = event.pageY - main_canvas_rect.top;
				temp.forEach(
					(element) => console.log(element)
				);
				
				tempValues.set(temp);
				device.queue.writeBuffer(temp_StorageBuffer, 0, tempValues);
				render();
				message_output_update(command_output, command_history, '請輸入第二個點:');
				line_state++;
			}
			else
			{
				temp[2] = event.pageX - main_canvas_rect.left;
				temp[3] = event.pageY - main_canvas_rect.top;
				temp.forEach(
					(element) => console.log(element)
				);
				
				line_insert(temp);
				tempValues.set(temp);
				device.queue.writeBuffer(temp_StorageBuffer, 0, tempValues);
				render();
				message_output_update(command_output, command_history, '完成');
				line_state = 0;
				
				input_mode = 'normal';
				mode_display.innerText = "模式: normal";
				console.log('normal mode');
			}
		}
	}
}

function line_insert(temp){
	for(let i = 0; i < all_geometry_data.line.length; i++){

		if(all_geometry_data.line[i].space > 0){
			for(let j = 0; j < page_size / 8; j++){
				if((all_geometry_data.line[i].manage.value[j] & 1) == 0){
					all_geometry_data.line[i].manage.value[j] |= 1;
					
					all_geometry_data.line[i].begin.value[j*2] = temp[0];
					all_geometry_data.line[i].begin.value[j*2 + 1] = temp[1];
					all_geometry_data.line[i].begin.submit();
					
					all_geometry_data.line[i].end.value[j*2] = temp[2];
					all_geometry_data.line[i].end.value[j*2 + 1] = temp[3];
					all_geometry_data.line[i].end.submit();

					console.log(all_geometry_data.line[i].manage.value);
					return;
				}

			}
		}
	}

	var new_space = new line_tensor();
	new_space.manage.value[0] |= 1;

	new_space.begin.value[0] = temp[0];
	new_space.begin.value[1] = temp[1];
	new_space.begin.submit();
	
	new_space.end.value[0] = temp[2];
	new_space.end.value[1] = temp[3];
	new_space.end.submit();

	console.log(new_space.manage.value);
	all_geometry_data.line.push(new_space);
}

window.onload = function()
{
	//ui -----------------------------------------------------------------------------------
	screen_width = document.getElementById("screen_width_html");
	screen_width.innerText = "寬度:" + window.innerWidth;
	
	ID_table = document.getElementById("ID_html");
			
	mode_display = document.getElementById("mode_display_html");
	//websocket -----------------------------------------------------------------------------
	websocket_connection = document.getElementById("websocket_connection_html");
	websocket_connection.addEventListener("click", websocket_connect_function);
	
	chat_send = document.getElementById("chat_send_html");
	chat_send.addEventListener("click", chat_send_function);

	chat_input = document.getElementById("chat_input_html");
	chat_output = document.getElementById("chat_output_html");

	//command -----------------------------------------------------------------------------	
	command_send = document.getElementById("command_send_html");
	command_send.addEventListener("click", command_send_function);

	command_input = document.getElementById("command_input_html");
	command_output = document.getElementById("command_output_html");
	
	//webgpu -----------------------------------------------------------------------------------
	main_canvas = document.getElementById("main_canvas_html");
	//canvas 的真實範圍(可接收點擊事件)要拉成跟外框一樣大
	main_canvas.width = main_canvas.offsetWidth - 2;
	main_canvas.height = main_canvas.offsetHeight - 2;
	
	main_canvas_rect = main_canvas.getBoundingClientRect();
	main_canvas_mouse_location = document.getElementById("main_canvas_mouse_location_html");
	main_canvas.addEventListener("mousemove", function(event) {
		//console.log(event.pageX - main_canvas_rect.left , event.pageY - main_canvas_rect.top);
		main_canvas_mouse_location.innerText = "滑鼠座標:" + (event.pageX - main_canvas_rect.left) + " " + (event.pageY - main_canvas_rect.top);
	});
	
	webgpu_init();
}




function websocket_connect_function()
{
	socket = new WebSocket("wss://lyciih.idv.tw:1223/");
	socket.onopen = function()
	{
		console.log("connect");
		websocket_connection.textContent = "斷線";
		websocket_connection.removeEventListener("click", websocket_connect_function);
		websocket_connection.addEventListener("click", websocket_close_function);
		socket.send("i");
	};

	socket.onmessage = function(event)
	{
		console.log(event.data);
		if(event.data.charAt(0) == "c")
		{
			message_output_update(chat_output, chat_history, event.data.substring(1));
		}
		if(event.data.charAt(0) == "i")
		{
			console.log(event.data);
			ID = event.data.substring(1);
			ID_table.innerText = "用戶ID:" + ID;
			all_client_data[ID].polyline.set([0.5, 0.5], 0);
			console.log(all_client_data[ID].polyline);
			render();
		}
	};


	socket.onclose = function(event)
	{
		console.log("disconnect");
		websocket_connection.textContent = "連線";
		websocket_connection.removeEventListener("click", websocket_close_function);
		websocket_connection.addEventListener("click", websocket_connect_function);
	}
}



function websocket_close_function()
{
	socket.close(1000, "正常斷開");
	console.log("close");
}

function chat_send_function()
{
	var message = "c" + chat_input.value;
	socket.send(message);
	chat_input.value = "";
}

function command_send_function()
{
	if(command_count < 6)
	{
		command_history.push(command_input.value);
		command_count++;
	}
	else
	{
		command_history.shift();
		command_history.push(command_input.value);
	}
	command_output.value = '';
	for(let i = 0; i < command_count; i++)
	{
		command_output.value = command_output.value + command_history[i] + '\n';
	}
	
	command_input.value = "";
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


	adapter = await navigator.gpu.requestAdapter();
	if(!adapter)
	{
		throw Error("Couldn't request webgpu adapter.");
	}

	device = await adapter.requestDevice();
	if(!device)
	{
		throw Error("Couldn't request webgpu device.");
	}

	console.log(adapter.limits);


	const main_context = main_canvas.getContext("webgpu");
	const main_format = navigator.gpu.getPreferredCanvasFormat();

	main_context.configure({
		device, 
		format: main_format,
	});


	const main_module = device.createShaderModule({
		label: 'temp',
		code:`

		struct size {
			width: i32,
			height: i32,
		}
		
		struct Vertex {
			x: i32,
			y: i32,
		}
		

		@group(0) @binding(0) var<uniform> canvas_size: size;
		@group(0) @binding(1) var<storage, read> begin: array<Vertex>;
		@group(0) @binding(2) var<storage, read> end: array<Vertex>;

		@vertex fn vs(
			@builtin(vertex_index) vertexIndex : u32,
			) -> @builtin(position) vec4f {

			var transform_x:f32;
			var transform_y:f32;

			if((vertexIndex & 1) == 0){
				transform_x = ((f32(begin[vertexIndex >> 1].x * 2)) / f32(canvas_size.width)) - 1;
				transform_y = 1 - ((f32(begin[vertexIndex >> 1].y * 2)) / f32(canvas_size.height));
				return vec4f(transform_x, transform_y, 0.0, 1.0);

			}
			else
			{
				transform_x = ((f32(end[vertexIndex >> 1].x * 2)) / f32(canvas_size.width)) - 1;
				transform_y = 1 - ((f32(end[vertexIndex >> 1].y * 2)) / f32(canvas_size.height));
				return vec4f(transform_x, transform_y, 0.0, 1.0);
			}
		}

		@fragment fn fs() -> @location(0) vec4f {
			return vec4f(1.0, 0.0, 0.0, 0.0);
		}
		`,
	});

	pipeline = device.createRenderPipeline({
		label: 'temp pipeline',
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
			//topology: 'line-strip',
			topology: 'line-list',
		},
	});


	canvas_size_unitformBuffer = device.createBuffer({
		label: `canvas size uniform`,
		size: 8,
		usage: GPUBufferUsage.UNIFORM | GPUBufferUsage.COPY_DST,
	});
	const canvas_size_uniformValues = new Int32Array(2);
	canvas_size_uniformValues[0] = main_canvas.width;
	canvas_size_uniformValues[1] = main_canvas.height;
	device.queue.writeBuffer(canvas_size_unitformBuffer, 0, canvas_size_uniformValues);



	temp_StorageBuffer = device.createBuffer({
		label: `storage for temp`,
		size: 1024,
		usage: GPUBufferUsage.STORAGE | GPUBufferUsage.COPY_DST,
	});
	tempValues = new Int32Array(1024 / 4);
	tempValues.set(temp);
	device.queue.writeBuffer(temp_StorageBuffer, 0, tempValues);


	
	const renderPassDescriptor = {
		label: 'canvas renderPass',
		colorAttachments: [
			{
				clearValue: [0.3, 0.3, 0.3, 1],
				loadOp: 'clear',	//load 和 clear 兩種選項
				storeOp: 'store',
			},
		],
	};


	all_geometry_data = new geometry_data();


	render = function() {
		//為了在每次canvas改變大小時能重新取得繪製尺寸，在這裡才取得尺寸，而不是在 renderPassDescriptor 中先定義
		renderPassDescriptor.colorAttachments[0].view = main_context.getCurrentTexture().createView();
		

		const encoder = device.createCommandEncoder({label: 'encoder'});
		const pass = encoder.beginRenderPass(renderPassDescriptor);
		pass.setPipeline(pipeline);
		for(let i = 0; i < all_geometry_data.line.length; i++){
			pass.setBindGroup(0, all_geometry_data.line[i].bindGroup);
			pass.draw(page_size / 8 * 2);
		}
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

		main_canvas_rect = main_canvas.getBoundingClientRect();


		canvas_size_uniformValues[0] = main_canvas.width;
		canvas_size_uniformValues[1] = main_canvas.height;
		device.queue.writeBuffer(canvas_size_unitformBuffer, 0, canvas_size_uniformValues);

		tempValues.set(temp);
		device.queue.writeBuffer(temp_StorageBuffer, 0, tempValues);
		render();
	}
	
	addEventListener("resize", screen_resize);

	main_canvas.width = main_canvas.offsetWidth - 2;
	main_canvas.height = main_canvas.offsetHeight - 2;
	render();
}

