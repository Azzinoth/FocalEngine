import sys, io, datetime
import requests

from mcp.server.fastmcp import FastMCP
mcp = FastMCP('focalengine')

@mcp.tool()
def get_start_scene_id() -> str | None:
    """Get the current start scene ID from FocalEngine"""
    response = requests.post('http://localhost:5187/api/get_start_scene_id', 
                            json={})
    return response.json()['scene_id']

# Need to fix issue with multi threading in FocalEngine before enabling this
#@mcp.tool()
#def screenshot_scene(scene_id: str) -> dict:
#    """Render scene with ID to a PNG on disk and return the file path.
#    Args:
#        scene_id: Scene ID.
#    Returns:
#        {'success': bool, 'path': 'C:/...', 'error': '...'} 
#    """
#    payload = {'scene_id': scene_id}
#    r = requests.post('http://localhost:5187/api/screenshot_scene', json=payload)
#    return r.json()

@mcp.tool()
def get_entities(scene_id: str) -> dict:
    """Get all entities in a scene with their IDs and names"""
    response = requests.post('http://localhost:5187/api/get_entities', 
                            json={'scene_id': scene_id})
    data = response.json()
    return {
        'count': data['count'],
        'entities': data['entities']
    }

@mcp.tool()
def get_entity_position(scene_id: str, entity_id: str) -> dict:
    """Get an entity's world position (x, y, z)
    
    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to query

    Returns:
        Dictionary with 'x', 'y', 'z' coordinates with 'success' boolean and optional 'error' message
    """
    response = requests.post(
        'http://localhost:5187/api/get_entity_position',
        json={'scene_id': scene_id, 'entity_id': entity_id}
    )
    data = response.json()
    return data

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