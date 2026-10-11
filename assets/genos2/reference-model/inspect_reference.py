import bpy,json,os
from pathlib import Path
from mathutils import Vector
OUT=Path(__file__).resolve().parent
deps=bpy.context.evaluated_depsgraph_get(); items=[]
for o in bpy.data.objects:
    if not any(c.name.startswith(('01_','02_','03_','04_','05_','06_')) for c in o.users_collection) or o.type not in ('MESH','FONT'):continue
    eo=o.evaluated_get(deps); m=eo.to_mesh(); m.calc_loop_triangles()
    bbox=[o.matrix_world@Vector(v) for v in o.bound_box]
    items.append({'name':o.name,'type':o.type,'collections':[c.name for c in o.users_collection],'renderHidden':o.hide_render,'vertices':len(m.vertices),'triangles':len(m.loop_triangles),'uvLayers':[u.name for u in m.uv_layers],'materials':[x.name if x else None for x in m.materials],'location':list(o.matrix_world.translation),'bboxMin':[min(v[i] for v in bbox) for i in range(3)],'bboxMax':[max(v[i] for v in bbox) for i in range(3)],'properties':{k:o[k] for k in o.keys()}})
    eo.to_mesh_clear()
materials=[]
for m in bpy.data.materials:
    textures=[{'image':n.image.name if n.image else None,'uv':n.uv_map if hasattr(n,'uv_map') else None,'interpolation':n.interpolation,'extension':n.extension} for n in m.node_tree.nodes if n.type=='TEX_IMAGE'] if m.node_tree else []
    materials.append({'name':m.name,'diffuse':list(m.diffuse_color),'textures':textures})
result={'source':bpy.data.filepath,'unitScale':bpy.context.scene.unit_settings.scale_length,'objects':items,'triangles':sum(x['triangles'] for x in items),'meshCount':sum(x['type']=='MESH' for x in items),'materials':materials,'images':[{'name':i.name,'size':list(i.size),'packed':bool(i.packed_file),'filepath':i.filepath,'channels':i.channels} for i in bpy.data.images]}
(OUT/'source-inventory.json').write_text(json.dumps(result,indent=2,ensure_ascii=False),encoding='utf8')
print('INVENTORY',result['meshCount'],result['triangles'],result['images'],flush=True)
