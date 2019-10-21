var ws;

var rexminiData;

function onLoad() {
		ws = new WebSocket("ws://roboterarm:9090/websocket");

		ws.onmessage = function(e) {

			tmpStr = e.data.replace(/\n/g, "<br />");
			if(tmpStr!='') {
				rexminiData = tmpStr.split(" ");
			}

		};
}


function buttonDown(value) {
	ws.send(value);
}


$(document).ready(function() {

	onLoad();

	setInterval(function(){ buttonDown('m'); }, 200);

	$('.btn_controll').bind('touchstart mousedown',function(event)
	{
		event.preventDefault();
		thisValue = $(this).val();
		thisAtr = $(this).attr('btnD');
		// $('#out').append(thisAtr);
		$('#status'+thisValue).css("background-color","blue");
		buttonDown(thisAtr);
	});

	$('.btn_controll').bind('touchend touchcancel mouseup',function(event)
	{
		event.preventDefault();
		thisValue = $(this).val();
		thisAtr = $(this).attr('btnU');
		// $('#out').append(thisAtr);
		$('#status'+thisValue).css("background-color","white");
		buttonDown(thisAtr);
	});

});
