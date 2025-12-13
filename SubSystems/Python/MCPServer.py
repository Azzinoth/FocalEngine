from http.server import HTTPServer, BaseHTTPRequestHandler
import json
import FocalEnginePython as FocalEngine


import base64
# pip install Pillow
from PIL import Image
import io
import os

def image_to_base64_compressed(image_path: str, max_size_kb: int = 950) -> str:
    """
    Convert an image to base64, compressing if necessary to stay under size limit.
    
    Args:
        image_path: Path to the PNG image file
        max_size_kb: Maximum size in kilobytes (default 950KB)
    
    Returns:
        tuple: base64_string
    """
    max_size_bytes = max_size_kb * 1024
    
    # Need to compress - open with PIL
    img = Image.open(image_path)
    
    # Convert RGBA to RGB if necessary (for JPEG compatibility)
    if img.mode in ('RGBA', 'LA', 'P'):
        background = Image.new('RGB', img.size, (255, 255, 255))
        if img.mode == 'P':
            img = img.convert('RGBA')
        background.paste(img, mask=img.split()[-1] if img.mode in ('RGBA', 'LA') else None)
        img = background
    
    # Start with high quality JPEG
    quality = 95
    
    while quality >= 5:  # Don't go below quality 5
        buffer = io.BytesIO()
        img.save(buffer, format='JPEG', quality=quality, optimize=True)
        
        # Get base64 encoded data
        jpeg_data = buffer.getvalue()
        jpeg_base64 = base64.b64encode(jpeg_data).decode('utf-8')
        
        # Check size
        if len(jpeg_base64.encode('utf-8')) <= max_size_bytes:
            return jpeg_base64
        
        # Reduce quality for next iteration
        quality -= 5
    
    # If still too large, try resizing
    scale_factor = 0.9
    while scale_factor >= 0.3:  # Don't shrink more than 70%
        new_width = int(img.width * scale_factor)
        new_height = int(img.height * scale_factor)
        resized_img = img.resize((new_width, new_height), Image.Resampling.LANCZOS)
        
        buffer = io.BytesIO()
        resized_img.save(buffer, format='JPEG', quality=85, optimize=True)
        
        jpeg_data = buffer.getvalue()
        jpeg_base64 = base64.b64encode(jpeg_data).decode('utf-8')
        
        if len(jpeg_base64.encode('utf-8')) <= max_size_bytes:
            return jpeg_base64
        
        scale_factor -= 0.1
    
    # Return best effort (shouldn't normally reach here)
    return jpeg_base64

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
		
		elif self.path == '/api/get_entity_name':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					EntityName = Entity.GetName()
					Result = {'success': True, 'name': EntityName}
				else:
					Result = {'success': False, 'error': 'Entity not found'}
			else:
				Result = {'success': False, 'error': 'Scene not found'}
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps(Result).encode())

		elif self.path == '/api/set_entity_name':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			NewName = data.get('name', '')
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						Entity.SetName(NewName)
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

		elif self.path == '/api/set_entity_rotation':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			pitch = data.get('pitch', 0.0)
			yaw = data.get('yaw', 0.0)
			roll = data.get('roll', 0.0)
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						Entity.SetRotation(FocalEngine.Vector3(pitch, yaw, roll))
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

		elif self.path == '/api/get_entity_rotation':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')

			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						Rotation = Entity.GetRotation()
						Result = {
							'success': True,
							'rotation': {'pitch': float(Rotation.x), 'yaw': float(Rotation.y), 'roll': float(Rotation.z)}
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

		elif self.path == '/api/get_entity_scale':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						Scale = Entity.GetScale()
						Result = {
							'success': True,
							'scale': {'x': float(Scale.x), 'y': float(Scale.y), 'z': float(Scale.z)}
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

		elif self.path == '/api/set_entity_scale':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			x = data.get('x', 1.0)
			y = data.get('y', 1.0)
			z = data.get('z', 1.0)
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						Entity.SetScale(FocalEngine.Vector3(x, y, z))
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

		elif self.path == '/api/get_parent_entity':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						ParentEntity = Entity.GetParent()
						if ParentEntity is not None:
							ParentID = ParentEntity.GetID()
						else:
							ParentID = None
						Result = {
							'success': True,
							'parent_entity_id': ParentID
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

		elif self.path == '/api/attach_to_entity':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			ChildEntityID = data.get('entity_id')
			ParentEntityID = data.get('parent_entity_id')
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				ChildEntity = Scene.get_entity(ChildEntityID)
				ParentEntity = Scene.get_entity(ParentEntityID)
				if ChildEntity is not None and ParentEntity is not None:
					try:
						ChildEntity.AttachTo(ParentEntity)
						Result = {'success': True}
					except Exception as Error:
						Result = {'success': False, 'error': str(Error)}
				else:
					Result = {'success': False, 'error': 'One or both entities not found'}
			else:
				Result = {'success': False, 'error': 'Scene not found'}
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps(Result).encode())

		elif self.path == '/api/detach_from_entity':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						Entity.Detach()
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

		elif self.path == '/api/is_child_of':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			ChildEntityID = data.get('entity_id')
			ParentEntityID = data.get('parent_entity_id')
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				ChildEntity = Scene.get_entity(ChildEntityID)
				ParentEntity = Scene.get_entity(ParentEntityID)
				if ChildEntity is not None and ParentEntity is not None:
					try:
						IsChild = ChildEntity.IsChildOf(ParentEntity)
						Result = {
							'success': True,
							'is_child': IsChild
						}
					except Exception as Error:
						Result = {'success': False, 'error': str(Error)}
				else:
					Result = {'success': False, 'error': 'One or both entities not found'}
			else:
				Result = {'success': False, 'error': 'Scene not found'}
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps(Result).encode())

		elif self.path == '/api/get_child_entities':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			ParentEntityID = data.get('entity_id')
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				ParentEntity = Scene.get_entity(ParentEntityID)
				if ParentEntity is not None:
					try:
						ChildEntities = ParentEntity.GetChildEntities()
						ChildEntityIDs = [child.GetID() for child in ChildEntities]
						Result = {
							'success': True,
							'child_entity_ids': ChildEntityIDs
						}
					except Exception as Error:
						Result = {'success': False, 'error': str(Error)}
				else:
					Result = {'success': False, 'error': 'Parent entity not found'}
			else:
				Result = {'success': False, 'error': 'Scene not found'}
			self.send_response(200)
			self.send_header('Content-type', 'application/json')
			self.end_headers()
			self.wfile.write(json.dumps(Result).encode())

		elif self.path == '/api/get_entity_instance_seed':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						InstanceSeed = Entity.GetInstanceSeed()
						Result = {
							'success': True,
							'instance_seed': InstanceSeed
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

		elif self.path == '/api/set_entity_instance_seed':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			NewSeed = data.get('seed', 0)
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						Entity.SetInstanceSeed(NewSeed)
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

		elif self.path == '/api/get_entity_instance_count':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						InstanceCount = Entity.GetInstanceCount()
						Result = {
							'success': True,
							'instance_count': InstanceCount
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

		elif self.path == '/api/set_entity_instance_count':
			SceneManager = FocalEngine.SceneManager.instance
			SceneID = data.get('scene_id')
			EntityID = data.get('entity_id')
			NewCount = data.get('count', 1)
			Scene = SceneManager.get_scene(SceneID)
			if Scene is not None:
				Entity = Scene.get_entity(EntityID)
				if Entity is not None:
					try:
						Entity.SetInstanceCount(NewCount)
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

		elif self.path == '/api/capture_screenshot':
			import base64
			try:
				sm = FocalEngine.SceneManager.instance
				scene = sm.get_starting_scene()

				# Signal to C++ that we want a screenshot
				FocalEngine.set_render_flag(True)

				# Wait for C++ to capture screenshot and clear the flag
				FocalEngine.wait_for_screenshot()
        
				if scene is None:
					result = {'success': False, 'error': 'No active scene'}
				else:
					# Assume C++ saved the screenshot to a known path
					screenshot_path = "D:\\test.png"
					success = True
            
					if success:
						image_base64 = image_to_base64_compressed(screenshot_path, max_size_kb=950)

						# For debugging: save the compressed image to verify
						debug_output_path = "D:\\temp_screenshot_compressed.jpg"
						with open(debug_output_path, 'wb') as f:
							f.write(base64.b64decode(image_base64))
                
						result = {
							'success': True,
							'type': 'image',
							'data': image_base64,
							'mimeType': 'image/jpeg'
						}
					else:
						result = {'success': False, 'error': 'Screenshot capture failed'}

			except Exception as e:
				result = {'success': False, 'error': str(e)}
				# Make sure to clear flag on error
				FocalEngine.set_render_flag(False)
    
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