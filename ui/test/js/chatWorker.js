// chat worker


onmessage = function(e){
	switch(e.data.type){
		case 'chat':
			console.log("chat worker receive: ",e.data.data);
			postMessage("I am chat worker");
			break;
		default:
			console.log("unknow type", e.data.type);
	}			
};

