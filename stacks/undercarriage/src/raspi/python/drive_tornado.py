#!/usr/bin/env python3.7
import tornado.web
import tornado.websocket
import tornado.httpserver
import tornado.ioloop
import serial
import threading
import asyncio

ser = serial.Serial('/dev/ttyUSB0', 38400, timeout=1)
print(ser.name)

clients = []


class ThreadReadSerial(threading.Thread):
	running = 1
	def run(self):
		asyncio.set_event_loop(asyncio.new_event_loop())
		while self.running:
			try:
				global ser
				data = str(ser.readline()).replace('\n','')
				if data!="b''":
					c.write_message("test")
					data.encode('utf8')
					print(data)
					# print('from Arduino: ', data
					# received from Arduino written to all WebSocket clients
					global clients
					for c in clients:
						c.write_message("test")
						c.write_message(data)
			except Exception as e:
				print('error1'+str(e))
			except serial.SerialException as es:
				print('error ser '+str(es))



class WebSocketHandler(tornado.websocket.WebSocketHandler):
		connections = set()

		def check_origin(self, origin):
			return True

		def open(self):
				self.connections.add(self)
				self.set_nodelay(True);
				# ser.write("1");
				# ser.write("m");
				global clients
				clients.append(self)
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
				global clients
				clients.remove(self)
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

	ws_app = Application()
	server = tornado.httpserver.HTTPServer(ws_app)
	server.listen(9090)
	serreadthread = ThreadReadSerial()
	serreadthread.start()
	tornado.ioloop.IOLoop.instance().start()
