from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import FocalEnginePython as FocalEngine

class EngineAPIHandler(BaseHTTPRequestHandler):
	def do_POST(self):
		content_length = int(self.headers['Content-Length'])
		post_data = self.rfile.read(content_length)
		data = json.loads(post_data)

		if self.path == '/api/get_start_scene_id':
			sm = FocalEngine.SceneManager.instance
			scene = sm.get_starting_scene()
			if scene is not None:
				scene_id = scene.get_ID()
			else:
				scene_id = None
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps({'scene_id': scene_id}).encode())
		
		# Get scene by ID
		elif self.path == '/api/get_scene':
			sm = FocalEngine.SceneManager.instance
			scene_id = data.get('scene_id')
			scene = sm.get_scene(scene_id)
			result = {'scene_id': scene.get_ID()} if scene is not None else {'scene_id': None}
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps(result).encode())

		elif self.path == '/api/get_entities':
			sm = FocalEngine.SceneManager.instance
			scene_id = data.get('scene_id')
			scene = sm.get_scene(scene_id)
			if scene is not None:
				entity_ids = scene.get_entity_id_list()
				entities = []
				for entity_id in entity_ids:
					entity = scene.get_entity(entity_id)
					if entity is not None:
						entities.append({
							'id': entity.GetID(),
							'name': entity.GetName()
						})
			else:
				entities = []
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps({'entities': entities, 'count': len(entities)}).encode())

		# for resource-based approach
		#elif self.path == '/api/get_entities':
		#	sm = FocalEngine.SceneManager.instance
		#	scene_id = data.get('scene_id')
		#	scene = sm.get_scene(scene_id)
		#	if scene is not None:
		#		entity_ids = scene.get_entity_id_list()
		#		entities = []
		#		for entity_id in entity_ids:
		#			entity = scene.get_entity(entity_id)
		#			if entity is not None:
		#				entities.append({
		#					'ID': entity.GetID(),
		#					'Name': entity.GetName()
		#				})
		#	else:
		#		entities = []
		#	self.send_response(200)
		#	self.send_header('Content-type', 'application/json')
		#	self.end_headers()
		#	self.wfile.write(json.dumps({'Entities': entities, 'Count': len(entities)}).encode())
		
		# Set entity position (absolute)
		elif self.path == '/api/set_entity_position':
			sm = FocalEngine.SceneManager.instance
			scene_id = data.get('scene_id')
			entity_id = data.get('entity_id')
			x = data.get('x', 0.0)
			y = data.get('y', 0.0)
			z = data.get('z', 0.0)
			scene = sm.get_scene(scene_id)
			if scene is not None:
				entity = scene.get_entity(entity_id)
				if entity is not None:
					try:
						entity.SetPosition(FocalEngine.Vector3(x, y, z))
						result = {'success': True}
					except Exception as e:
						result = {'success': False, 'error': str(e)}
				else:
					result = {'success': False, 'error': 'Entity not found'}
			else:
				result = {'success': False, 'error': 'Scene not found'}
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps(result).encode())

		else:
			self.send_response(404)
			self.end_headers()

def start_server(port=5187):
	import datetime
	try:
		server = HTTPServer(('localhost', port), EngineAPIHandler)

		print(f"Engine API server running on port {port}")
		server.serve_forever()
		
	except Exception as e:
		try:
			with open('D:\\server_error.txt', 'w') as f:
				f.write(f'Error type: {type(e).__name__}\n')
				f.write(f'Error message: {str(e)}\n')
				import traceback
				f.write(traceback.format_exc())
				f.flush()
		except:
			# If even error writing fails, try a simple file
			with open('D:\\critical_error.txt', 'w') as f:
				f.write('Failed to write error details\n')
				f.flush()

import threading
server_thread = threading.Thread(target=start_server)
server_thread.start()

import time
time.sleep(0.5)