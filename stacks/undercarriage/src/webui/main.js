var ws;

var rexminiData;

var spaceshuttle_data = new Object()

function onLoad() {
		console.log('init websocket')


		ws = new WebSocket("ws://10.0.0.254:9090/websocket");

		ws.onmessage = function(e) {

			// console.log(e)

			tmp = e.data.split(":")

			if(tmp[0]=='batt')
			{
				// console.log(tmp[1])
				var battpercentage = 0;
				spaceshuttle_data.batt = tmp[1]
				// console.log(spaceshuttle_data)
				battpercentage = (spaceshuttle_data.batt/25000)*100
				battpercentage = Math.round(battpercentage)
				$("#battval").html(battpercentage+'%')
			}

		};

		ws.onopen = function(e) {
			console.log('websocket connection open')
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

		if(thisAtr!='')
		{
			sendWebsocketCommand(thisAtr)
		}
	});

});
