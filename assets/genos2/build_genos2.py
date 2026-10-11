"""Rebuild the reference-derived GENOS2 facade. Blender 5.2; no external assets."""
import bpy, math, json, os
from mathutils import Vector
OUT=os.path.dirname(os.path.abspath(__file__))
bpy.ops.object.select_all(action='SELECT'); bpy.ops.object.delete(use_global=False)
materials={}; controls=[]; labels=[]; screens=[]
PALETTE={'body':'242A2D','panel':'30373B','rubber':'101315','ivory':'ECEBE5','black':'15191B','blue':'4B9CEB','orange':'FF9A30','silver':'949FA5','text':'D0D8DC','glass':'101D29','cyan':'69CAE9','screen':'142E43'}
def mat(name):
    if name not in materials:
        h=PALETTE.get(name,name); c=tuple(int(h[i:i+2],16)/255 for i in (0,2,4))
        linear=tuple(v/12.92 if v<=.04045 else ((v+.055)/1.055)**2.4 for v in c)
        m=bpy.data.materials.new(name); m.diffuse_color=(*linear,1); m.use_nodes=True
        p=m.node_tree.nodes.get('Principled BSDF'); p.inputs['Base Color'].default_value=(*linear,1); p.inputs['Roughness'].default_value=.48 if name not in ('ivory','glass','silver') else .3
        p.inputs['Specular IOR Level'].default_value=.18 if name in ('body','panel','rubber','black') else .35
        if name in ('blue','orange','cyan'):
            p.inputs['Emission Color'].default_value=(*c,1); p.inputs['Emission Strength'].default_value=.25
        materials[name]=m
    return materials[name]
def own(o,name,material,control='',layer=0):
    o.name=name; o.data.materials.append(mat(material)); o['control']=control; o['layer']=layer
    o['moving']=not name.startswith(('fader:channel','fader:slot','fader:led'))
    return o
def cube(name,c,w,d,h,material,control='',bevel=.018,layer=0):
    # Contract coordinates x/y-up/z-depth -> Blender x/y-depth/z-up.
    bpy.ops.mesh.primitive_cube_add(size=1,location=(c[0],-c[2],c[1]+h/2)); o=bpy.context.object
    o.dimensions=(w,d,h); bpy.ops.object.transform_apply(location=False,rotation=False,scale=True)
    if bevel:
        b=o.modifiers.new('physical edge radius','BEVEL'); b.width=min(bevel,w/3,d/3,h/3); b.segments=1
        bpy.ops.object.modifier_apply(modifier=b.name)
        n=o.modifiers.new('weighted material normals','WEIGHTED_NORMAL')
    return own(o,name,material,control,layer)
def cylinder(name,c,r,h,material,control='',vertices=16,layer=2):
    bpy.ops.mesh.primitive_cylinder_add(vertices=vertices,radius=r,depth=h,location=(c[0],-c[2],c[1]+h/2))
    return own(bpy.context.object,name,material,control,layer)
def text(t,c,size=.075,colour='text',align='CENTER'):
    bpy.ops.object.text_add(location=(c[0],-c[2],c[1])); o=bpy.context.object; o.name='label:'+t
    o.data.body=t; o.data.size=size; o.data.align_x=align; o.data.align_y='CENTER'; o.data.space_character=1.1; o.data.materials.append(mat(colour)); o['label']=True
    labels.append({'text':t,'position':list(c),'size':size,'colour':int('FF'+PALETTE.get(colour,colour),16)})
    return o
def control(cid,kind,label,x,z,w,d,h=.065,value=0,black=False,hint=''):
    y=.475 if kind!='key' else (.38 if black else .225)
    controls.append(dict(id=cid,kind=kind,label=label,hint=hint,centre=[x,y,z],width=w,depth=d,height=h,value=value,black=black))
    return (x,y,z)
def button(cid,label,x,z,w=.165,d=.13,active=False):
    c=control(cid,'button',label,x,z,w,d,hint=label)
    cube('button:'+cid,c,w,d,.065,'blue' if active else 'silver',cid,.022,4)
    if label: text(label,(x,.551,z-.14),.049)
def deco_button(name,x,z,w=.16,d=.13,material='rubber',label=''):
    cube('decor:'+name,(x,.475,z),w,d,.052,material,bevel=.018,layer=2)
    if label: text(label,(x,.539,z-.12),.042)
def rounded_footprint(w,d,r,steps=6):
    points=[]
    for cx,cy,a in ((w/2-r,d/2-r,0),(-w/2+r,d/2-r,90),(-w/2+r,-d/2+r,180),(w/2-r,-d/2+r,270)):
        for i in range(steps+1):
            angle=math.radians(a+i*90/steps); points.append((cx+r*math.cos(angle),cy+r*math.sin(angle)))
    return points
def rounded_case(name,w,d,r,bottom,top,material,zoffset=0):
    xy=rounded_footprint(w,d,r); n=len(xy); v=[(x,-z-zoffset,y) for y in (bottom,top) for x,z in xy]
    # x, depth, height Blender coordinates.
    faces=[tuple(reversed(range(n))),tuple(range(n,2*n))]+[(i,(i+1)%n,(i+1)%n+n,i+n) for i in range(n)]
    faces=[tuple(reversed(f)) for f in faces]
    mesh=bpy.data.meshes.new(name); mesh.from_pydata(v,[],faces); mesh.update(); o=bpy.data.objects.new(name,mesh); bpy.context.collection.objects.link(o)
    bpy.context.view_layer.objects.active=o; o.select_set(True)
    b=o.modifiers.new('molded case edge radius','BEVEL'); b.width=.05; b.segments=3
    bpy.ops.object.modifier_apply(modifier=b.name); o.select_set(False)
    return own(o,name,material)

# Molded case: lower rounded tub, separate keyboard recess and panel, curved rear rail.
rounded_case('case:lower rounded tub',12.4,4.45,.32,0,.22,'body')
rounded_case('case:raised control deck',12.34,2.8,.27,.20,.445,'panel',-.765)
cube('case:keyboard recess',(0,.16,1.375),11.45,1.56,.035,'rubber',bevel=.035)
cube('case:left cheek',(-5.73,.2,1.35),.92,1.54,.23,'body',bevel=.095)
cube('case:right cheek',(5.86,.2,1.35),.58,1.54,.23,'body',bevel=.095)
cube('case:front lip',(0,.135,2.115),11.8,.17,.105,'black',bevel=.042)
cube('case:curved back cap',(0,.445,-2.045),11.9,.24,.13,'body',bevel=.065)
cube('case:central live-control ridge',(-2.45,.445,-1.92),2.78,.21,.13,'body',bevel=.055)
cube('case:display ridge',(.06,.445,-1.80),2.45,.39,.12,'body',bevel=.055)
# Keyboard end markers and sculpted individual key volumes.
first,last=28,103
isblack=lambda n:n%12 in (1,3,6,8,10)
whitecount=sum(not isblack(n) for n in range(first,last+1)); keyw=10.48/whitecount; start=-5.02; wi=0
for n in range(first,last+1):
    black=isblack(n); x=start+(wi if black else wi+.5)*keyw
    d=.84 if black else 1.40; z=.68+d/2
    c=control('note'+str(n),'key','',x,z,keyw*(.59 if black else .969),d,.155 if black else .118,n,black,'Jouer MIDI '+str(n))
    cube('key:'+str(n),c,keyw*(.59 if black else .969),d,.155 if black else .118,'black' if black else 'ivory','note'+str(n),.013,5)
    if black: cube('keytop:'+str(n),(x,.532,z-.025),keyw*.48,d*.91,.003,'rubber','note'+str(n),.005,5)
    if not black: wi+=1
# Single joystick in the left key cheek (X pitch / Z modulation).
cube('joystick:recess',(-5.64,.435,1.15),.56,.61,.014,'rubber',bevel=.06,layer=2)
cube('joystick:side surround',(-5.64,.45,1.15),.44,.40,.04,'black',bevel=.03,layer=2)
c=control('pitch','joystick','PITCH / MOD',-5.64,1.15,.37,.34,.13,0,False,'Glisser les deux axes ; pitch recentre au relachement')
cube('joystick:one lever',c,.30,.12,.13,'rubber','pitch',.035,4)
text('JOYSTICK',(-5.64,.454,.73),.058)
# Main and live control displays, using original Cadenza-only screen content.
main={'id':'main','centre':[.00,.48,-.98],'width':2.12,'depth':1.06}; live={'id':'live','centre':[-2.56,.48,-1.60],'width':1.74,'depth':.18}; screens=[main,live]
cube('screen:main bezel',(0,.451,-.98),2.28,1.23,.026,'rubber',bevel=.025,layer=1)
cube('screen:main glass',tuple(main['centre']),main['width'],main['depth'],.010,'screen',bevel=.009,layer=2)
cube('screen:live bezel',(-2.56,.451,-1.60),1.86,.28,.039,'rubber',bevel=.025,layer=1)
cube('screen:live glass',tuple(live['centre']),live['width'],live['depth'],.010,'glass',bevel=.006,layer=2)
for i,name in enumerate(('REV','COMP','VOL','LOW','MID','HIGH')):
    x=-3.30+i*.296
    text(name,(x,.497,-1.65),.052,'cyan'); text('100',(x,.497,-1.55),.053,'text')
    c=control('knob'+str(i),'knob',name,x,-1.12,.155,.155,.142,i,False,'Regler '+name)
    cylinder('rotary:collar'+str(i),(x,.449,-1.12),.096,.025,'silver',vertices=12)
    cylinder('rotary:'+str(i),c,.075,.142,'black','knob'+str(i),vertices=16,layer=4)
    cube('rotary:pointer'+str(i),(x,.619,-1.157),.015,.065,.008,'orange','knob'+str(i),.002,4)
    text(str(i+1),(x,.458,-1.32),.047)
for i in range(9):
    x=-3.30+i*.184
    c=control('fader'+str(i),'fader',str(i+1),x,-.57,.094,.58,.05,i,False,'Volume partie '+str(i+1))
    cube('fader:channel'+str(i),(x,.45,-.57),.13,.62,.024,'rubber','fader'+str(i),bevel=.01,layer=4)
    cube('fader:slot'+str(i),(x,.477,-.57),.015,.56,.004,'black','fader'+str(i),bevel=0,layer=4)
    for k in range(7): cube('fader:led'+str(i)+'_'+str(k),(x+.036,.480,-.38-k*.042),.014,.020,.003,'orange','fader'+str(i),bevel=0,layer=4)
    cube('fader:cap'+str(i),(x,.49,-.65),.097,.12,.06,'silver','fader'+str(i),.012,4)
    text(str(i+1),(x,.481,-.20),.048)
text('LIVE CONTROL',(-2.54,.452,-1.90),.07)
# Original live screen artwork, text excluded from runtime baked faces.
cube('screen:header',(0,.493,-1.36),1.95,.19,.002,'glass',bevel=0,layer=2)
text('CADENZA',(-.88,.502,-1.38),.068,'cyan','LEFT'); text('120 BPM',(.88,.502,-1.38),.064,'text','RIGHT')
text('Ballad / Style A',(-.85,.502,-1.10),.092,'text','LEFT')
for i in range(4):
    x=-.72+i*.47; cube('screen:part'+str(i),(x,.493,-.89),.42,.25,.002,'glass',bevel=0,layer=2)
    text(('LEFT','R1','R2','R3')[i],(x,.502,-.93),.052,'cyan'); text(('Piano','Strings','Guitar','Organ')[i],(x,.502,-.82),.045)
cube('screen:status',(0,.493,-.62),1.95,.15,.002,'orange',bevel=0,layer=2)
text('MAIN A   |   READY',(0,.502,-.62),.061,'black')
text('STYLE   MIXER   EDITOR',(0,.502,-.47),.047)
# Left MIDI/audio/tempo area occupies the matching physical banks.
control('master','knob','MASTER',-5.44,-1.17,.27,.27,.103,2,False,'Regler le volume maitre Cadenza')
cylinder('rotary:master dial',(-5.44,.475,-1.17),.132,.103,'black','master',vertices=20,layer=4)
cube('rotary:master pointer',(-5.44,.579,-1.244),.023,.085,.005,'orange','master',bevel=.003,layer=4)
text('MASTER VOLUME',(-5.44,.455,-1.42),.052)
button('midi','MIDI',-4.88,-1.33,.18,.13)
button('audio','AUDIO',-4.59,-1.33,.18,.13)
button('rec','REC',-4.88,-1.02,.18,.13)
deco_button('song stop',-4.59,-1.02,label='SONG')
button('style','STYLE',-4.90,-.72,.25,.13)
button('transpose-','-',-5.67,-.19); button('transpose+','+',-5.44,-.19)
text('TRANSPOSE',(-5.55,.456,-.41),.047)
button('tempo-','-',-4.83,-.41); button('tempo+','+',-4.60,-.41)
text('TEMPO',(-4.72,.456,-.59),.049)
deco_button('tap tempo',-4.35,-.41,label='TAP')
button('acmp','ACMP',-4.95,.23,.175,.13,True)
button('memory','HOLD',-4.72,.23,.175,.13,True)
button('autofill','AUTO',-4.48,.23,.175,.13)
for i,cid in enumerate(('introA','introB','introC')): button(cid,('I','II','III')[i],-4.15+i*.21,.23,.165,.13,False)
for i in range(4): button('main'+chr(65+i),chr(65+i),-3.40+i*.22,.23,.172,.13,i==0)
button('fill','FILL',-2.47,.23,.16,.13,True)
button('break','BRK',-2.27,.23,.16,.13)
for i,cid in enumerate(('endingA','endingB','endingC')): button(cid,('I','II','III')[i],-2.05+i*.21,.23,.165,.13)
button('syncstop','SYNC',-1.28,.23,.20,.13)
button('play','START',-1.01,.23,.21,.13,True)
text('STYLE CONTROL',(-3.18,.455,.50),.062)
# Registration memory is physically present, but clearly decorative in this build.
for i in range(8): deco_button('registration '+str(i+1),-.55+i*.197,.23,.15,.13,'silver',str(i+1))
text('REGISTRATION MEMORY',(.16,.455,.50),.058)
for i in range(4): button('ots'+str(i),str(i+1),1.68+i*.195,.23,.155,.13,True)
text('ONE TOUCH SETTING',(1.975,.455,.50),.052)
for i in range(4): deco_button('multipad '+str(i+1),2.75+i*.195,.23,.155,.13,'blue',str(i+1))
text('MULTI PAD CONTROL',(3.04,.455,.50),.052)
for i in range(3): button('right'+str(i),'R'+str(i+1),4.0+i*.20,.23,.15,.13,i==0)
button('left','L',4.68,.23,.15,.13)
text('PART ON / OFF',(4.30,.455,.50),.052)
# Encoder beside the screen; decorative until a specific application action exists.
cylinder('decor:data encoder',(1.70,.47,-1.22),.195,.072,'black',vertices=24)
cylinder('decor:data encoder metal',(1.70,.54,-1.22),.169,.007,'silver',vertices=24)
cube('decor:encoder mark',(1.70,.55,-1.34),.025,.07,.006,'black',bevel=0,layer=2)
text('DATA',(1.70,.455,-1.52),.062)
button('creator','EDITOR',1.72,-.39,.19,.13)
button('native','CADENZA',2.55,-1.40,.23,.13)
button('import','IMPORT',2.93,-1.40,.23,.13)
button('soundfont','SONS',2.55,-1.08,.23,.13)
button('save','SAVE',2.93,-1.08,.23,.13)
deco_button('assignable 1',2.55,-.76,.23,.13,'blue','ASSIGN')
deco_button('assignable 2',2.93,-.76,.23,.13,'blue','')
# Right speaker-like touch strip recess and minimal physical inscriptions.
cube('decor:right touch recess',(5.07,.449,-1.03),.43,.67,.014,'rubber',bevel=.045,layer=1)
cube('decor:right touch strip',(5.07,.465,-1.03),.35,.54,.008,'black',bevel=.030,layer=2)
text('CADENZA',(.02,.458,-.20),.125,'text')
text('Genos2', (5.19,.459,-.12),.19,'silver')
text('REFERENCE FACADE',(5.12,.459,-.34),.047,'silver')
for x in (-5.61,5.57):
    cylinder('decor:rear fastener '+str(x),(x,.585,-2.02),.040,.004,'silver',vertices=8)

# Export all evaluated mesh geometry (labels intentionally remain separate).
deps=bpy.context.evaluated_depsgraph_get(); faces=[]; count=0
for ob in bpy.context.scene.objects:
    if ob.type!='MESH': continue
    evaluated=ob.evaluated_get(deps); mesh=evaluated.to_mesh(); mesh.calc_loop_triangles()
    for tri in mesh.loop_triangles:
        pts=[ob.matrix_world @ mesh.vertices[i].co for i in tri.vertices]
        v=[[round(p.x,6),round(p.z,6),round(-p.y,6)] for p in pts]
        n=(pts[1]-pts[0]).cross(pts[2]-pts[0]).normalized()
        h=PALETTE.get(ob.data.materials[tri.material_index].name,ob.data.materials[tri.material_index].name)
        rgb=[int(h[i:i+2],16) for i in (0,2,4)]
        # Directional baked material shading maintains readable physical facets on CPU.
        factor=.72+.28*max(0,n.dot(Vector((-.35,-.6,1)).normalized()))
        rgb=[min(255,round(c*factor)) for c in rgb]
        colour=(255<<24)|(rgb[0]<<16)|(rgb[1]<<8)|rgb[2]
        faces.append({'vertices':v,'colour':colour,'control':ob.get('control',''),'top':bool(n.z>.45),'layer':int(ob.get('layer',0)),'moving':bool(ob.get('moving',True))})
    evaluated.to_mesh_clear()
if len(faces)>20000: raise RuntimeError('CPU geometry budget exceeded: '+str(len(faces)))
payload={'version':1,'coordinates':'x horizontal / y up / z depth; rear negative','source':'build_genos2.py / Blender 5.2','faces':faces,'controls':controls,'labels':labels,'screen':main,'screens':screens,'statistics':{'triangles':len(faces),'keys':76,'rotaries':6,'masterRotaries':1,'faders':9,'joysticks':1}}
with open(os.path.join(OUT,'geometry.json.tmp'),'w',encoding='utf-8') as f: json.dump(payload,f,separators=(',',':'),ensure_ascii=False)
os.replace(os.path.join(OUT,'geometry.json.tmp'),os.path.join(OUT,'geometry.json'))
print('GEOMETRY_READY',len(faces),len(controls),flush=True)

# Actual Blender preview setup, isolated from any existing interactive instance.
scene=bpy.context.scene; scene.render.engine='CYCLES'; scene.cycles.samples=24
scene.cycles.use_denoising=True; scene.render.resolution_x=1700; scene.render.resolution_y=900; scene.render.resolution_percentage=100
scene.world.color=(.8,.8,.8)
world=scene.world; world.use_nodes=True; world.node_tree.nodes['Background'].inputs[0].default_value=(.82,.85,.88,1); world.node_tree.nodes['Background'].inputs[1].default_value=.50
scene.view_settings.view_transform='AgX'
cube('render:floor',(0,-.18,0),200,200,.1,'E6E8EB',bevel=0)
for name,loc,power,size in [('key',(-4,-1,8),1100,7),('fill',(4,4,6),700,5),('rim',(0,6,4),650,6)]:
    bpy.ops.object.light_add(type='AREA',location=loc); l=bpy.context.object; l.name='render:'+name; l.data.energy=power; l.data.shape='DISK'; l.data.size=size
    l.rotation_euler=(Vector((0,0,0))-l.location).to_track_quat('-Z','Y').to_euler()
bpy.ops.object.camera_add(location=(-8.5,-10.5,9.5)); camera=bpy.context.object; camera.name='camera:perspective'; scene.camera=camera
camera.rotation_euler=(Vector((0,0,.3))-camera.location).to_track_quat('-Z','Y').to_euler(); camera.data.type='ORTHO'; camera.data.ortho_scale=15.4; camera.data.lens=45
scene.render.image_settings.file_format='PNG'; scene.render.filepath=os.path.join(OUT,'preview-perspective.png')
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT,'genos2.blend'))
bpy.ops.render.render(write_still=True)
camera.location=(0,0,16); camera.rotation_euler=(Vector((0,0,0))-camera.location).to_track_quat('-Z','Y').to_euler(); camera.data.ortho_scale=13.4
scene.render.resolution_y=660; scene.render.filepath=os.path.join(OUT,'preview-top.png'); bpy.ops.render.render(write_still=True)
print('RENDERS_READY',flush=True)
