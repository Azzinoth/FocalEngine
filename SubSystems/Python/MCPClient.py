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

from mcp import types

@mcp.tool()
def capture_scene_screenshot() -> list[types.TextContent | types.ImageContent]:
    """Capture a screenshot of the current scene in FocalEngine.
    
    Use this to see the current state of the scene before making decisions
    about entity placement, movements, or other modifications.
    
    Returns:
        Base64 encoded JPEG image of the current scene
    """
    response = requests.post('http://localhost:5187/api/capture_screenshot', json={})
    data = response.json()
    
    if data['success']:
        return [
            types.ImageContent(
                type="image",
                data=data['data'],
                mimeType="image/jpeg"
            )
        ]
    else:
        return [
            types.TextContent(
                type="text",
                text=f"Error capturing screenshot: {data.get('error', 'Unknown error')}"
            )
        ]

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