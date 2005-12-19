#!BPY

"""
Name: 'OpenGL (.js)...'
Blender: 232
Group: 'Export'
Tooltip: 'Save to a JavaScript source file'
"""

#####
#####  BLENDER TO OPENGL EXPORTER
#####
#####  2003 - Bruce "Sinner" Barrera
#####
#####  contact: sinner@opengl.com.br
#####
#####  2004 - Raphael Langerhorst
#####   + transformation
#####   + material
#####   + exporting of ALL meshes
#####
#####  contact: raphael-langerhorst@gmx.at
#####
#####  2005 - Jens Thiele
#####   + modified to export javascript opengl code for egachine
#####
#####  contact: karme@berlios.de
#####
#####
#####  LAST UPDATED: 19 Dec 2005
#####
#####  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
#####

##### 
##### Notes from raphael:
##### 
##### * Scaling and deformations:
#####   These are not handled in the script, you MUST select all objects
#####   of the scene and select
#####   Transform -> Clear/Apply -> Apply Size/Rotation  (Ctrl+A)
#####   Transform -> Clear/Apply -> Apply Deformation  (Shift+Ctrl+A)
#####   before exporting in order to have correct transformations
#####   in the exported file.
##### 
##### * ### NAO MODIFIQUE NADA DAKI PRA BAIXO, I don't know what this means
#####   (it's from the original author, I just left it in)
#####
##### * Tested against Blender 2.32
#####


#==================================================#
# New name based on old with a different extension #
#==================================================#
def newFName(ext):
	return Blender.Get('filename')[: -len(Blender.Get('filename').split('.', -1)[-1]) ] + ext

import Blender
from Blender import Object, NMesh
	
def save_opengl(filename):
	
	## Open file
	f = open(filename,"w")
	
	print "File %s created and opened. Now exporting..." % filename
	
	## Which object to export
	#currently all objects (meshes only - see below)
	objects = [ob for ob in Object.GetSelected() if ob.getType() == 'Mesh']
	
	index = 0
	for obj in objects:
		nmesh = NMesh.GetRawFromObject(obj.name)
	
		f.write("\n\n//object: %s_%d\n" % (nmesh.name, index) )
	
		f.write("function load_%d() {\n" % index)
		index += 1
		
		#used for materials
		f.write("\tvar matColors=[];\n");
		
		#transformation data
		
		#first translate, then rotate, then scale
		# because they are applied from "back to front"
		
		#we also store the current matrix and restore it
		# after the object is rendered (push, pop)
		f.write("\tgl.PushMatrix();\n")
		
		#translation
		
		locx = 0;
		locy = 0;
		locz = 0;
		if obj.LocX > 0.0001 or obj.LocX < -0.0001:
			locx = obj.LocX
		if obj.LocY > 0.0001 or obj.LocY < -0.0001:
			locy = obj.LocY
		if obj.LocZ > 0.0001 or obj.LocZ < -0.0001:
			locz = obj.LocZ
		
		f.write("\tgl.Translated(%.6f, %.6f, %.6f);\n" % (locx, locy, locz))
		
		#rotation - this seems to be buggy !?
		if obj.RotX > 0.0001 or obj.RotX < -0.0001:
			f.write("  gl.Rotated(%.6f,1,0,0);\n" % (-obj.RotX*180*0.31831))
		if obj.RotY > 0.0001 or obj.RotY < -0.0001:
			f.write("  gl.Rotated(%.6f,0,1,0);\n" % (-obj.RotY*180*0.31831))
		if obj.RotZ > 0.0001 or obj.RotZ < -0.0001:
			f.write("  gl.Rotated(%.6f,0,0,1);\n" % (-obj.RotZ*180*0.31831))
		
		#scaling
		f.write("\tgl.Scaled(%.6f, %.6f, %.6f);\n" % tuple(obj.size))
		
		tricount = 0
		lastMaterialIndex = -1;
		for face in nmesh.faces:
			vn = 0
			tricount = tricount + 1
			  
			#material - can be changed between glBegin and glEnd
			if nmesh.materials:
				if face.materialIndex != lastMaterialIndex:
					lastMaterialIndex = face.materialIndex
					material = nmesh.materials[face.materialIndex]
					print "exporting face material"
					
					#ambient and diffuse
					f.write("\tmatColors[0] = %.6f;\n" % material.R)
					f.write("\tmatColors[1] = %.6f;\n" % material.G)
					f.write("\tmatColors[2] = %.6f;\n" % material.B)
					f.write("\tmatColors[3] = %.6f;\n" % material.alpha)
					f.write("\tgl.Materialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,matColors);\n")
					
					#specular
					specs = material.getSpecCol()
					f.write("\tmatColors[0] = %.6f;\n" % specs[0])
					f.write("\tmatColors[1] = %.6f;\n" % specs[1])
					f.write("\tmatColors[2] = %.6f;\n" % specs[2])
					f.write("\tmatColors[3] = %.6f;\n" % material.getSpecTransp())
					f.write("\tgl.Materialfv(GL_FRONT_AND_BACK,GL_SPECULAR,matColors);\n")
			
			print "Exporting %s triangle(s)" % tricount  
			
			if len(face) == 3: f.write("\tgl.Begin(GL_TRIANGLES);\n")
			elif len(face) == 4: f.write("\tgl.Begin(GL_QUADS);\n")
			  
			for vertex in face.v:
				## Build glVertex3f
				f.write("\tgl.Normal3f(%.6f, %.6f, %.6f);\n" % tuple(vertex.no))
				f.write("\tgl.Vertex3f(%.6f, %.6f, %.6f);\n" % tuple(vertex.co))
				vn+=1
			#print "glEnd();"
			f.write("\tgl.End();\n")
			
		f.write("\tgl.PopMatrix();\n")
		f.write("}\n")
	
	f.write("\n\n")
	
	#print "void loadModel() {"
	f.write("function loadModel() {\n")
	
	index = 0
	
	for obj in objects:
		f.write("\tload_%d();" % index)
		f.write("\t//object: %s\n" % obj.getData())
		index += 1
	
	f.write("}\n")
	
	print "Export complete"
	
	f.close()


Blender.Window.FileSelector(save_opengl, 'Export JavaScript OpenGL', newFName('js'))
