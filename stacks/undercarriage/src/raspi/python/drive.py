#!/usr/bin/env python3.7
import tornado.web
import tornado.websocket
import tornado.httpserver
import tornado.ioloop
import serial
import threading
import asyncio
import time



clients = []
serialready = True


class ThreadReadSerial(threading.Thread):
	running = 1
	serialbuff = ""
	def __init__(self):
		threading.Thread.__init__(self)
		self.stop_event = threading.Event()
		self.serialbuff = ""


	def run(self):
		global clients
		global ser
		asyncio.set_event_loop(asyncio.new_event_loop())
		while not self.stop_event.is_set():
			try:
				data = ser.read().decode()
				if len(data)!=0:
					if data!='\n' and data!='\r' and data!='>':
						self.serialbuff = self.serialbuff + data
					if data == '\n' and self.serialbuff!="":
						for c in clients:
							c.write_message(self.serialbuff)
						self.serialbuff = ""
			except Exception as e:
				print('error1'+str(e))
			except serial.SerialException as es:
				print('error ser '+str(es))

class ThreadBroadcastHealth(threading.Thread):
	def __init__(self):
		threading.Thread.__init__(self)
		self.stop_event = threading.Event()

	def run(self):
		global ser
		while not self.stop_event.is_set():
			try:
				time.sleep(2)
				battinforequest = bytearray('v\n', 'utf-8')
				ser.write(battinforequest)
				# print('request bat info')
			except Exception as e:
				print('error health '+str(e))


class WebSocketHandler(tornado.websocket.WebSocketHandler):
		connections = set()

		def check_origin(self, origin):
			return True

		def open(self):
				self.connections.add(self)
				self.set_nodelay(True)
				# ser.write("1");
				# ser.write("m");
				global clients
				clients.append(self)
				print('new connection was opened')
				pass

		def on_message(self, message):
				global ser
				#print('from WebSocket: ', message
				b = bytearray(message+'\n', 'utf-8')
				try:
					ser.write(b)     # received from WebSocket writen to arduino
					ser.flushInput()
					ser.reset_input_buffer()
					ser.reset_output_buffer()

				# ser.write("ff?\n")
				except Exception as e:
					print('error2'+str(e))

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
	ser = serial.Serial('/dev/ttyUSB0', 38400, timeout=1)
	print(ser.name)

	ser.flushInput()
	ser.reset_input_buffer()
	ser.reset_output_buffer()

	ws_app = Application()
	server = tornado.httpserver.HTTPServer(ws_app)
	server.listen(9090)

	serreadthread = ThreadReadSerial()
	serreadthread.daemon = True
	serreadthread.start()

	readthreadhealth = ThreadBroadcastHealth()
	readthreadhealth.daemon = True
	readthreadhealth.start()

	tornado.ioloop.IOLoop.instance().start()
