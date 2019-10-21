#!/usr/bin/env python3.7
import tornado.web
import tornado.websocket
import tornado.httpserver
import tornado.ioloop
import serial
import threading

ser = serial.Serial('/dev/ttyUSB0', 38400, timeout=1)
print(ser.name)

def readSerial(ser):
		while True:
			try:
				data = ser.readline();
				# print('from Arduino: ', data
				# received from Arduino written to all WebSocket clients
				# [con.write_message(data) for con in WebSocketHandler.connections]
			except Exception:
				print('error1')


class WebSocketHandler(tornado.websocket.WebSocketHandler):
		connections = set()

		def check_origin(self, origin):
			return True

		def open(self):
				self.connections.add(self)
				self.set_nodelay(True);
				# ser.write("1");
				# ser.write("m");
				print('new connection was opened')
				pass

		def on_message(self, message):
				#print('from WebSocket: ', message
				b = bytearray(message, 'utf-8')
				try:
					ser.write(b);     # received from WebSocket writen to arduino
				# ser.write("ff?\n")
				except Exception:
					print('error2')

		def on_close(self):
				self.connections.remove(self)
				print('connection closed')
				pass


class Application(tornado.web.Application):
	def __init__(self):
		handlers = [
			(r'/websocket', WebSocketHandler),
			(r'/(.*)', tornado.web.StaticFileHandler, {'path': './root'})
		]

		settings = {
			'template_path': 'templates'
		}
		tornado.web.Application.__init__(self, handlers, **settings)

if __name__ == '__main__':
	ser.flushInput()
	thread = threading.Thread(target=readSerial, args=(ser,))
	thread.start()
	ws_app = Application()
	server = tornado.httpserver.HTTPServer(ws_app)
	server.listen(9090)
	tornado.ioloop.IOLoop.instance().start()
