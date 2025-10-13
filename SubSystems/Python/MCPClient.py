import sys, io, datetime
import requests

from mcp.server.fastmcp import FastMCP
mcp = FastMCP('focalengine')

#@mcp.resource("scene://{scene_id}/entities")
#async def get_entities_resource(scene_id: str) -> str:
#    """List of all entities in a scene with their IDs and names
    
#    URI format: scene://{scene_id}/entities
#    """
    # Parse the scene_id from the URI
    # URI format: scene://{scene_id}/entities
#    parts = uri.split('/')
#    if len(parts) < 4 or parts[0] != 'scene:':
#        return json.dumps({'error': 'Invalid URI format'})
    
#    scene_id = parts[2]
    
#    response = requests.post('http://localhost:5187/api/get_entities', 
#                            json={'scene_id': scene_id})
#    data = response.json()
    
    # Format as readable string for the LLM
#    result = f"Scene {scene_id} contains {data['Count']} entities:\n\n"
#    for entity in data['Entities']:
#        result += f"- ID: {entity['ID']}, Name: {entity['Name']}\n"

#    return result

@mcp.tool()
def get_start_scene_id() -> str | None:
    """Get the current start scene ID from FocalEngine"""
    response = requests.post('http://localhost:5187/api/get_start_scene_id', 
                            json={})
    return response.json()['scene_id']

@mcp.tool()
def get_entities(scene_id: str) -> dict:
    """Get all entities in a scene with their IDs and names"""
    response = requests.post('http://localhost:5187/api/get_entities', 
                            json={'scene_id': scene_id})
    data = response.json()
    return {
        'count': data['count'],
        'entities': data['entities']  # List of dicts with 'id' and 'name'
    }

@mcp.tool()
def move_entity(scene_id: str, entity_id: str, x: float, y: float, z: float) -> dict:
    """Move an entity to a specific position in world space
    
    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to move
        x: X coordinate in world space
        y: Y coordinate in world space
        z: Z coordinate in world space
    
    Returns:
        Dictionary with 'success' boolean and optional 'error' message
    """
    response = requests.post('http://localhost:5187/api/set_entity_position', 
                            json={
                                'scene_id': scene_id, 
                                'entity_id': entity_id,
                                'x': x,
                                'y': y,
                                'z': z
                            })
    return response.json()

if __name__ == "__main__":
    mcp.run()