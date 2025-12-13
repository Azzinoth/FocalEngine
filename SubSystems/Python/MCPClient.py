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
def get_entity_name(scene_id: str, entity_id: str) -> dict:
    """Get an entity's name

    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to query

    Returns:
        Dictionary with 'name' value with 'success' boolean and optional 'error' message
    """
    response = requests.post(
        'http://localhost:5187/api/get_entity_name',
        json={'scene_id': scene_id, 'entity_id': entity_id}
    )
    data = response.json()
    return data

@mcp.tool()
def set_entity_name(scene_id: str, entity_id: str, name: str) -> dict:
    """Set an entity's name

    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to modify
        name: The new name for the entity

    Returns:
        Dictionary with 'success' boolean and optional 'error' message
    """
    response = requests.post(
        'http://localhost:5187/api/set_entity_name',
        json={'scene_id': scene_id, 'entity_id': entity_id, 'name': name}
    )
    data = response.json()
    return data

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

@mcp.tool()
def get_entity_rotation(scene_id: str, entity_id: str) -> dict:
    """Get an entity's world rotation (pitch, yaw, roll)
    
    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to query
    Returns:
        Dictionary with 'pitch', 'yaw', 'roll' values with 'success' boolean and optional 'error' message
    """
    response = requests.post(
        'http://localhost:5187/api/get_entity_rotation',
        json={'scene_id': scene_id, 'entity_id': entity_id}
    )
    data = response.json()
    return data

@mcp.tool()
def set_entity_rotation(scene_id: str, entity_id: str, pitch: float, yaw: float, roll: float) -> dict:
    """Set an entity's world rotation (pitch, yaw, roll)
    
    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to rotate
        pitch: Pitch angle in degrees
        yaw: Yaw angle in degrees
        roll: Roll angle in degrees
    
    Returns:
        Dictionary with 'success' boolean and optional 'error' message
    """
    response = requests.post('http://localhost:5187/api/set_entity_rotation', 
                            json={
                                'scene_id': scene_id, 
                                'entity_id': entity_id,
                                'pitch': pitch,
                                'yaw': yaw,
                                'roll': roll
                            })
    return response.json()

@mcp.tool()
def get_entity_scale(scene_id: str, entity_id: str) -> dict:
    """Get an entity's world scale (x, y, z)
    
    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to query
    Returns:
        Dictionary with 'x', 'y', 'z' scale values with 'success' boolean and optional 'error' message
    """
    response = requests.post(
        'http://localhost:5187/api/get_entity_scale',
        json={'scene_id': scene_id, 'entity_id': entity_id}
    )
    data = response.json()
    return data

@mcp.tool()
def set_entity_scale(scene_id: str, entity_id: str, x: float, y: float, z: float) -> dict:
    """Set an entity's world scale (x, y, z)
    
    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to scale
        x: Scale factor along X axis
        y: Scale factor along Y axis
        z: Scale factor along Z axis
    
    Returns:
        Dictionary with 'success' boolean and optional 'error' message
    """
    response = requests.post('http://localhost:5187/api/set_entity_scale', 
                            json={
                                'scene_id': scene_id, 
                                'entity_id': entity_id,
                                'x': x,
                                'y': y,
                                'z': z
                            })
    return response.json()

@mcp.tool()
def get_parent_entity(scene_id: str, entity_id: str) -> dict:
    """Get an entity's parent entity ID

    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to query

    Returns:
        Dictionary with 'parent_entity_id' value with 'success' boolean and optional 'error' message
    """
    response = requests.post(
        'http://localhost:5187/api/get_parent_entity',
        json={'scene_id': scene_id, 'entity_id': entity_id}
    )
    data = response.json()
    return data

@mcp.tool()
def attach_to_entity(scene_id: str, entity_id: str, parent_entity_id: str) -> dict:
    """Attach an entity to a parent entity
    Args:
        scene_id: The ID of the scene containing the entities
        entity_id: The ID of the entity to attach
        parent_entity_id: The ID of the parent entity to attach to

    Returns:
        Dictionary with 'success' boolean and optional 'error' message
    """
    response = requests.post('http://localhost:5187/api/attach_to_entity',
                             json={
                                 'scene_id': scene_id,
                                 'entity_id': entity_id,
                                 'parent_entity_id': parent_entity_id
                             })
    return response.json()

@mcp.tool()
def detach_from_entity(scene_id: str, entity_id: str) -> dict:
    """Detach an entity from its parent entity
    Args:
        scene_id: The ID of the scene containing the entities
        entity_id: The ID of the entity to detach

    Returns:
        Dictionary with 'success' boolean and optional 'error' message
    """
    response = requests.post('http://localhost:5187/api/detach_from_entity',
                             json={
                                 'scene_id': scene_id,
                                 'entity_id': entity_id
                             })
    return response.json()

@mcp.tool()
def is_child_of(scene_id: str, entity_id: str, parent_entity_id: str) -> dict:
    """Check if an entity is a child of a specific parent entity
    Args:
        scene_id: The ID of the scene containing the entities
        entity_id: The ID of the entity to check
        parent_entity_id: The ID of the potential parent entity

    Returns:
        Dictionary with 'success' boolean and optional 'error' message
    """
    response = requests.post('http://localhost:5187/api/is_child_of',
                             json={
                                 'scene_id': scene_id,
                                 'entity_id': entity_id,
                                 'parent_entity_id': parent_entity_id
                             })
    return response.json()

@mcp.tool()
def get_child_entities(scene_id: str, entity_id: str) -> dict:
    """Get a list of child entities for a specific entity
    Args:
        scene_id: The ID of the scene containing the entities
        entity_id: The ID of the entity to query

    Returns:
        Dictionary with 'child_entities' list and optional 'error' message
    """
    response = requests.post('http://localhost:5187/api/get_child_entities',
                             json={
                                 'scene_id': scene_id,
                                 'entity_id': entity_id
                             })
    return response.json()

@mcp.tool()
def get_entity_instance_seed(scene_id: str, entity_id: str) -> dict:
    """Get an entity's instance seed value
    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to query
    Returns:
        Dictionary with 'seed' value with 'success' boolean and optional 'error' message
    """
    response = requests.post(
        'http://localhost:5187/api/get_entity_instance_seed',
        json={'scene_id': scene_id, 'entity_id': entity_id}
    )
    data = response.json()
    return data

@mcp.tool()
def set_entity_instance_seed(scene_id: str, entity_id: str, seed: int) -> dict:
    """Set an entity's instance seed value
    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to modify
        seed: The new seed value for the entity
    Returns:
        Dictionary with 'success' boolean and optional 'error' message
    """
    response = requests.post(
        'http://localhost:5187/api/set_entity_instance_seed',
        json={'scene_id': scene_id, 'entity_id': entity_id, 'seed': seed}
    )
    data = response.json()
    return data

@mcp.tool()
def get_entity_instance_count(scene_id: str, entity_id: str) -> dict:
    """Get an entity's instance count value
    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to query
    Returns:
        Dictionary with 'count' value with 'success' boolean and optional 'error' message
    """
    response = requests.post(
        'http://localhost:5187/api/get_entity_instance_count',
        json={'scene_id': scene_id, 'entity_id': entity_id}
    )
    data = response.json()
    return data

@mcp.tool()
def set_entity_instance_count(scene_id: str, entity_id: str, count: int) -> dict:
    """Set an entity's instance count value
    Args:
        scene_id: The ID of the scene containing the entity
        entity_id: The ID of the entity to modify
        count: The new instance count value for the entity
    Returns:
        Dictionary with 'success' boolean and optional 'error' message
    """
    response = requests.post(
        'http://localhost:5187/api/set_entity_instance_count',
        json={'scene_id': scene_id, 'entity_id': entity_id, 'count': count}
    )
    data = response.json()
    return data

if __name__ == "__main__":
    mcp.run()