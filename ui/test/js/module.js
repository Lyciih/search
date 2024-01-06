import { gv } from './global_variable.js';



function message_output_update(content_interface, content, string){
	// 將顯示的欄位滾動到最下方
	content_interface.scrollTop = content_interface.scrollHeight;

	// 判斷訊息陣列是否需要平移
	if(content.count < 500){
		content.value.push(string);
		content.count++;
	}
	else{
		content.value.shift();
		content.value.push(string);
	}

	// 清空欄位中原有的訊息
	content_interface.value = '';

	// 按順序加入訊息與換行
	for(let i = 0; i < content.count; i++){
		content_interface.value = content_interface.value + content.value[i] + '\n';
	}
}





export const system_check = {
	webgl: webgl_support_check,
	webgpu: webgpu_support_check,
	Worker: Worker_support_check,
	SharedWorker: SharedWorker_support_check
}



// 檢查 WebGPU 是否支援的函數
function webgpu_support_check(){
	if(navigator.gpu){
		console.log("WebGPU is supported");
		message_output_update(gv.system_output, gv.system_history, "WebGPU is supported")
		return 1;
	}
	else{
		console.log("WebGPU is not supported");
		message_output_update(gv.system_output, gv.system_history, "WebGPU is not supported")
		return 0;
	}
}


// 檢查 WebGL 是否支援的函數
function webgl_support_check(){
	if(!!window.WebGLRenderingContext){
		console.log("WebGL is supported");
		message_output_update(gv.system_output, gv.system_history, "WebGL is supported")
		return 1;
	}
	else{
		console.log("WebGL is not supported");
		message_output_update(gv.system_output, gv.system_history, "WebGL is not supported")
		return 0;
	}
}



// 檢查 SharedWorker 是否支援的函數
function SharedWorker_support_check(){
	if(!!window.SharedWorker){
		console.log("SharedWorker is supported");
		message_output_update(gv.system_output, gv.system_history, "SharedWorker is supported")
		return 1;
	}
	else{
		console.log("SharedWorker is not supported");
		message_output_update(gv.system_output, gv.system_history, "SharedWorker is not supported")
		return 0;
	}
}


// 檢查 Worker 是否支援的函數
function Worker_support_check(){
	if(!!window.Worker){
		console.log("Worker is supported");
		message_output_update(gv.system_output, gv.system_history, "Worker is supported")
		return 1;
	}
	else{
		console.log("Worker is not supported");
		message_output_update(gv.system_output, gv.system_history, "Worker is not supported")
		return 0;
	}
}
