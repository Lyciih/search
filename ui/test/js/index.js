import { gv } from './global_variable.js';
import { system_check } from './module.js';


window.onload = function(){
	// 網頁元素鍵接
	gv.state_bar = document.getElementById("state_bar_html");
	gv.system_output = document.getElementById("system_output_html");
	gv.websocket_connection = document.getElementById("websocket_connection_html");


	// 檢查圖形api支援性
	system_check.webgl();
	system_check.webgpu();
	system_check.Worker();
	system_check.SharedWorker();

	gv.chat_worker = new Worker('js/chatWorker.js');
	gv.chat_worker.onmessage = function(e){
		console.log("main receive: ", e.data);
	}
	gv.chat_worker.postMessage({ type: "chat", data: "hi chat worker" });

	
	gv.shareWorker = new SharedWorker('js/shareWorker.js');
	gv.share_port = gv.shareWorker.port;
	gv.websocket_connection.addEventListener("click", websocket_connect);

	gv.share_port.onmessage = function(e){
		console.log("main receive: ", e.data);
	}

	gv.share_port.postMessage({ type: "chat", data: "hi share worker" });


	gv.state_bar.innerText = "ID: ";

}


function websocket_connect(){
	gv.share_port.postMessage({type: "websocketConnect", data:"websocket"});
}
