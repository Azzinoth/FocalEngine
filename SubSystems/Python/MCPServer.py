from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import FocalEnginePython as FocalEngine

class EngineAPIHandler(BaseHTTPRequestHandler):
	def do_POST(self):
		content_length = int(self.headers['Content-Length'])
		post_data = self.rfile.read(content_length)
		data = json.loads(post_data)

		if self.path == '/api/get_start_scene_id':
			SceneManager = FocalEngine.SceneManager.instance
			Scene = SceneManager.get_starting_scene()
			if Scene is not None:
				SceneID = Scene.get_ID()
			else:
				SceneID = None
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps({'scene_id': SceneID}).encode())

		elif self.path == '/api/screenshot_scene':
			SceneID = data.get('scene_id', '')
			try:
				Saved = FocalEngine.create_screenshot(SceneID)
				Result = {'success': True, 'path': Saved}
			except Exception as e:
				Result = {'success': False, 'error': str(e)}

			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps(Result).encode())

		# Get scene by ID
		elif self.path == '/api/get_scene':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			Scene = SceneManager.get_scene(SceneID)
			result = {'scene_id': Scene.get_ID()} if Scene is not None else {'scene_id': None}
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps(result).encode())

		elif self.path == '/api/get_entities':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				EntityIDS = Scene.get_entity_id_list()
				Entities = []
				for EntityID in EntityIDS:
					Entity = Scene.get_entity(EntityID)
					if Entity is not None:
						Entities.append({
							'id': Entity.GetID(),
							'name': Entity.GetName()
						})
			else:
				Entities = []
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps({'entities': Entities, 'count': len(Entities)}).encode())
		
		# Set entity position (absolute)
		elif self.path == '/api/set_entity_position':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			x = data.get('x', 0.0)
			y = data.get('y', 0.0)
			z = data.get('z', 0.0)
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						Entity.SetPosition(FocalEngine.Vector3(x, y, z))
						Result = {'success': True}
					except Exception as Error:
						Result = {'success': False, 'error': str(Error)}
				else:
					Result = {'success': False, 'error': 'Entity not found'}
			else:
				Result = {'success': False, 'error': 'Scene not found'}
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps(Result).encode())

		elif self.path == '/api/get_entity_position':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')

			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						Position = Entity.GetPosition()
						Result = {
							'success': True,
							'position': {'x': float(Position.x), 'y': float(Position.y), 'z': float(Position.z)}
						}
					except Exception as Error:
						Result = {'success': False, 'error': str(Error)}
				else:
					Result = {'success': False, 'error': 'Entity not found'}
			else:
				Result = {'success': False, 'error': 'Scene not found'}

			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps(Result).encode())

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