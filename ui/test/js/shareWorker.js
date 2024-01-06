// share worker


var connect_port = [];


onconnect = function(event){
	var port = event.ports[0];

	connect_port.push(port);

	port.onmessage = function(e){
		switch(e.data.type){
			case "chat":
				console.log("share worker receive: ", e.data.data);
				port.postMessage("I am share worker");
				break;
			case "websocketConnect":
				console.log("share ", e.data.data);
				break;
			default:
				console.log("unknow type", e.data.type);
		}
	};

	// 某一個port被關閉的處理
	port.onclose = function(){
		// 過濾掉要刪除的port
		connect_port = connect_port.filter(function(p){
			return p !== port;
		});
	};
	
};


