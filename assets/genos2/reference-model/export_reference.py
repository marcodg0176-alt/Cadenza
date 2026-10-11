"""Export the approved Yamaha_Genos2.blend without editing the source file.
Run Blender --background SOURCE.blend --python export_reference.py.
"""
import bpy, json, math, struct, hashlib, shutil, re
from pathlib import Path
from mathutils import Vector
OUT=Path(__file__).resolve().parent
SOURCE=Path(bpy.data.filepath)
if SOURCE.name!='Yamaha_Genos2.blend': raise RuntimeError('Approved final model required')
def sha(p): return hashlib.sha256(Path(p).read_bytes()).hexdigest()
source_hash=sha(SOURCE)
SCALE=10.
def cv(p): return [float(p.x*SCALE),float(p.z*SCALE),float(-p.y*SCALE)]
def product(o): return o.type in ('MESH','FONT') and any(c.name.startswith(('01_','02_','03_','04_','05_','06_')) for c in o.users_collection)
deps=bpy.context.evaluated_depsgraph_get()
objects=sorted((o for o in bpy.data.objects if product(o)),key=lambda o:o.name)
cache={}
def mesh_for(o):
    if o.name not in cache:
        eo=o.evaluated_get(deps); m=eo.to_mesh(); m.calc_loop_triangles()
        coords=[o.matrix_world@v.co for v in m.vertices]
        lo=Vector([min(p[i] for p in coords) for i in range(3)]); hi=Vector([max(p[i] for p in coords) for i in range(3)])
        cache[o.name]=(eo,m,lo,hi)
    return cache[o.name]
controls=[]; binding={}; mapping=[]
def add(cid,kind,name,value=0,label='',hint='',members=None,axis=None,travel=None,neutral=None):
    o=bpy.data.objects[name]; _,m,lo,hi=mesh_for(o); centre=(lo+hi)*.5
    hit=Vector((centre.x,centre.y,hi.z+.000004))
    # Exact source handle rather than the middle of the travel slot.
    c={'id':cid,'kind':kind,'label':label or cid,'hint':hint or label or cid,
       'centre':cv(centre),'width':float((hi.x-lo.x)*SCALE),'depth':float((hi.y-lo.y)*SCALE),'height':float((hi.z-lo.z)*SCALE),
       'value':value,'black':o.get('key_color','')=='black','hitPoint':cv(hit),'pivot':cv(centre),'axis':axis or [0,1,0]}
    if travel is not None:c['travel']=travel;c['neutralValue']=neutral
    index=len(controls);controls.append(c)
    group=members or [(name,True)]
    for objname,moving in group:
        if objname not in bpy.data.objects:raise RuntimeError('Missing mesh '+objname)
        binding[objname]=(index,moving)
    mapping.append({'id':cid,'kind':kind,'sourceMesh':name,'meshes':[n for n,_ in group],'value':value})

for o in sorted((o for o in objects if 'midi_note' in o),key=lambda o:int(o['midi_note'])):
    n=int(o['midi_note']);add('note'+str(n),'key',o.name,n,label='',hint='Jouer la note MIDI '+str(n))
knob_labels=['REV','COMP','VOL','LOW','MID','HIGH']
for i in range(6):
    n=i+1
    members=[(o.name,not ('couronne' in o.name or o.name.startswith('LED encodeur'))) for o in objects if o.name.startswith('Encodeur LIVE CONTROL '+str(n)+' ') or o.name.startswith('Cannele encodeur '+str(n)+'-') or o.name.startswith('LED encodeur '+str(n)+'-')]
    add('knob'+str(i),'knob','Encodeur LIVE CONTROL '+str(n)+' capuchon',i,knob_labels[i],'Live Control '+str(n)+' : '+knob_labels[i],members)
add('master','knob','MASTER VOLUME',2,'MASTER','Regler le gain maitre Cadenza',[(o.name,'socle' not in o.name) for o in objects if o.name.startswith('MASTER VOLUME')])
for i in range(9):
    n=i+1; cap=bpy.data.objects['Fader '+str(n)+' curseur']; rail=bpy.data.objects['Fader '+str(n)+' glissiere']
    _,_,clo,chi=mesh_for(cap);_,_,rlo,rhi=mesh_for(rail)
    capCentre=(clo+chi)*.5; railCentre=(rlo+rhi)*.5
    course=float((rhi.y-rlo.y)-(chi.y-clo.y)); neutral=float((capCentre.y-(railCentre.y-course*.5))/course)
    # Original deck slope in this bank, transformed by the final dimensioned root.
    root=bpy.data.objects['YAMAHA GENOS2 - 1234 x 456 x 151 mm']
    slope=(.104-.093)/(.160+.065)*root.matrix_world.to_scale().z
    members=[(o.name,('curseur' in o.name or 'repere' in o.name)) for o in objects if o.name.startswith('Fader '+str(n)+' ') or o.name.startswith('LED fader '+str(n)+'-')]
    add('fader'+str(i),'fader',cap.name,i,str(n),'Volume Cadenza partie '+str(n),members,travel=[0,course*slope*SCALE,-course*SCALE],neutral=neutral)

buttons={
    'GATEWAY style voice 1':('style','STYLE'),
    'STYLE BREAK 1':('break','BREAK'),
    'STYLE commande 1':('syncstop','SYNC STOP'),
    'STYLE commande 3':('play','START / STOP'),
    'SONG autres 1':('acmp','ACMP'),
    'SONG autres 3':('autofill','AUTO FILL'),
    'TEMPO 1':('tempo-','TEMPO -'),'TEMPO 2':('tempo+','TEMPO +'),
    'TRANSPOSE 1':('transpose-','TRANSPOSE -'),'TRANSPOSE 2':('transpose+','TRANSPOSE +'),
    'SONG RECORD 1':('rec','REC'),
    'PART ON OFF 2':('left','LEFT'),
}
for i in range(3):buttons['STYLE INTRO '+str(i+1)]=('intro'+chr(65+i),('I','II','III')[i]);buttons['STYLE ENDING '+str(i+1)]=('ending'+chr(65+i),('I','II','III')[i]);buttons['PART ON OFF '+str(i+3)]=('right'+str(i),'R'+str(i+1))
for i in range(4):buttons['STYLE MAIN '+str(i+1)]=('main'+chr(65+i),chr(65+i));buttons['ONE TOUCH '+str(i+1)]=('ots'+str(i),str(i+1))
for name,(cid,label) in sorted(buttons.items()):
    add(cid,'button',name,0,label,'Commande Cadenza : '+label,[(name,True),(name+' embase',False)])
add('pitch','joystick','Bout du joystick',0,'PITCH / MOD','Joystick pitch et modulation',[(n,True) for n in ('Levier joystick','Bout du joystick','Axe transversal joystick')])

# Preserve both complete 2000x2000 packed image resources, without relabeling.
texture_defs=[('panel_texture_clean.png','panel.png','panel_png'),('Genos2_b_0001_tcm122-2176996.jpg','rear.png','rear_png')]
texture_indices={};textures=[]
for index,(name,filename,resource) in enumerate(texture_defs):
    im=bpy.data.images[name];raw=bytes(im.packed_file.data)
    if raw.startswith(b'\x89PNG\r\n\x1a\n'):(OUT/filename).write_bytes(raw)
    else:
        im.filepath_raw=str(OUT/filename);im.file_format='PNG';im.save()
    texture_indices[name]=index
    textures.append({'resource':resource,'file':filename,'width':im.size[0],'height':im.size[1],'sourceImage':name,'sha256':sha(OUT/filename),'addressMode':'clamp','filter':'bilinear'})

screen_objects={'Dalle LCD interface originale':'main','LIVE CONTROL VIEW ecran secondaire':'live'}
screens=[]
for name,sid in screen_objects.items():
    o=bpy.data.objects[name];m=o.data
    # Source quads were authored TL/TR/BR/BL and remain an inclined true plane.
    pts=[o.matrix_world@v.co for v in m.vertices[:4]]
    normal=(pts[1]-pts[0]).cross(pts[3]-pts[0]).normalized()
    if normal.z<0:normal=-normal
    pts=[p+normal*.000035 for p in pts]
    corners=[cv(p) for p in pts];centre=sum(pts,Vector())/4
    screens.append({'id':sid,'sourceMesh':name,'corners':corners,'centre':cv(centre),'width':(pts[1]-pts[0]).length*SCALE,'depth':(pts[3]-pts[0]).length*SCALE})

def srgb(v):return 12.92*v if v<=.0031308 else 1.055*v**(1/2.4)-.055
light=Vector((-.35,-.60,1)).normalized();count=0;uv_count=0;ledger=[]
tmp=OUT/'mesh.bin.tmp'
with tmp.open('wb') as stream:
    stream.write(b'GNOS2M01');stream.write(struct.pack('<I',0))
    for o in objects:
        if o.hide_render or o.name in screen_objects:continue
        _,m,lo,hi=mesh_for(o);cid,moving=binding.get(o.name,(-1,False));start=count
        uv_layer=next((u for u in m.uv_layers if u.active_render),m.uv_layers.active)
        for tri in m.loop_triangles:
            points=[o.matrix_world@m.vertices[v].co for v in tri.vertices]
            normal=(points[1]-points[0]).cross(points[2]-points[0]).normalized()
            material=m.materials[tri.material_index] if tri.material_index<len(m.materials) else None
            image_nodes=[n for n in material.node_tree.nodes if n.type=='TEX_IMAGE' and n.image] if material and material.node_tree else []
            texture=texture_indices.get(image_nodes[0].image.name,-1) if image_nodes else -1
            factor=(.80+.20*max(0,normal.dot(light))) if texture>=0 else (.65+.35*max(0,normal.dot(light)))
            if texture>=0:rgb=[min(255,round(255*factor))]*3
            else:
                colour=material.diffuse_color if material else (.021,.024,.027,1)
                rgb=[min(255,max(0,round(srgb(v)*255*factor))) for v in colour[:3]]
            argb=(255<<24)|(rgb[0]<<16)|(rgb[1]<<8)|rgb[2]
            uv=[(float(uv_layer.data[l].uv.x),float(1-uv_layer.data[l].uv.y)) for l in tri.loops] if texture>=0 and uv_layer else [(0,0)]*3
            verts=[value for p in points for value in cv(p)];uvs=[value for point in uv for value in point]
            stream.write(struct.pack('<15fIiiI',*verts,*uvs,argb,cid,texture,int(moving)))
            count+=1;uv_count+=texture>=0
        ledger.append({'name':o.name,'firstTriangle':start,'triangleCount':count-start,'control':controls[cid]['id'] if cid>=0 else '', 'moving':moving,'materials':[x.name if x else None for x in m.materials]})
    stream.seek(8);stream.write(struct.pack('<I',count))
tmp.replace(OUT/'mesh.bin')
meta={'version':2,'sourceModel':'Yamaha_Genos2.blend','sourceSha256':source_hash,'coordinates':'x right/y up/z front; source xyz -> x,z,-y *10','triangleResource':'mesh_bin','triangleFile':'mesh.bin','triangleCount':count,'binaryFormat':'GNOS2M01 uint32 count + <15fIiiI records (76 bytes)','controls':controls,'screens':screens,'labels':[],'textures':textures,'statistics':{'sourceMeshObjects':801,'sourceTriangles':205760,'runtimeTriangles':count,'texturedTriangles':uv_count,'commandCount':len(controls),'keys':76,'liveControlRotaries':6,'masterRotaries':1,'faders':9,'joysticks':1},'omittedRuntimeMeshes':list(screen_objects),'accessOutsideModel':{'toolbar':['creator','midi','audio','native'],'nativeUI':['import','soundfont','save','fill','memory']}}
(OUT/'geometry.json').write_text(json.dumps(meta,indent=2,ensure_ascii=False),encoding='utf8')
(OUT/'control-mapping.json').write_text(json.dumps(mapping,indent=2,ensure_ascii=False),encoding='utf8')
(OUT/'mesh-inventory.json').write_text(json.dumps(ledger,indent=2,ensure_ascii=False),encoding='utf8')
if SOURCE.resolve()!=(OUT/'Yamaha_Genos2.blend').resolve():shutil.copy2(SOURCE,OUT/'Yamaha_Genos2.blend')
if sha(SOURCE)!=source_hash or sha(OUT/'Yamaha_Genos2.blend')!=source_hash:raise RuntimeError('Source/copy hash mismatch')
for name in ('LIRE-MOI.txt',):
    if (SOURCE.parent.parent/name).is_file():shutil.copy2(SOURCE.parent.parent/name,OUT/'SOURCE-LIRE-MOI.txt')
for name in ('01_Perspective_hero.png','02_Dessus_orthographique.png','05_Detail_commandes.png'):
    if (SOURCE.parent.parent/'rendus'/name).is_file():shutil.copy2(SOURCE.parent.parent/'rendus'/name,OUT/name)
for eo,m,lo,hi in cache.values():eo.to_mesh_clear()
print('EXPORT_READY',json.dumps(meta['statistics']),flush=True)
print('CONTROL_IDS',','.join(c['id'] for c in controls if c['kind']!='key'),flush=True)
