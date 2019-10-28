var ws;

var rexminiData;

function onLoad() {
		console.log('init websocket')

		ws = new WebSocket("ws://10.0.0.254:9090/websocket");

		ws.onmessage = function(e) {

			// alert(e.data);
			console.log(e)

			/*
			tmpStr = e.data.replace(/\n/g, "<br />");
			if(tmpStr!='') {
				rexminiData = tmpStr.split(" ");
			}
			*/

		};

		ws.onopen = function(e) {
			console.log('websocket connection open')
			sendWebsocketCommand('v');
		}
}


function sendWebsocketCommand(value) {
	console.log('websocket sending '+value)
	ws.send(value);
}


$(document).ready(function() {

	if (typeof window.orientation !== 'undefined') {
		// alert('mobile')
	} else {
		// alert('desktop')
	}

	onLoad();

	// sendWebsocketCommand('v');

	// setInterval(function(){ sendWebsocketCommand('m'); }, 200);

	$('.btn_controll').bind('touchstart mousedown',function(event)
	{
		event.preventDefault();
		thisValue = $(this).val();
		thisAtr = $(this).attr('btnD');
		// $('#out').append(thisAtr);
		// $('#status'+thisValue).css("background-color","blue");
		sendWebsocketCommand(thisAtr);
	});

	$('.btn_controll').bind('touchend touchcancel mouseup',function(event)
	{
		event.preventDefault();
		thisValue = $(this).val();
		thisAtr = $(this).attr('btnU');
		// $('#out').append(thisAtr);
		// $('#status'+thisValue).css("background-color","white");
		sendWebsocketCommand(thisAtr);
	});

});
