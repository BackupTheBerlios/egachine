#!BPY

"""
Name: 'OpenGL (.c)...'
Blender: 232
Group: 'Export'
Tooltip: 'Save to a C source file'
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
#####  LAST UPDATED: 07 July 2005
#####
#####  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
#####

##### 
##### Notes from raphael:
##### 
##### * The filename is currently hardcoded (/home/raphael/blender_model.c)
#####   Simply change it to your needs inside the script (or write a
#####   dialog that asks for input - I'm no Python wizard - in fact this
#####   script is the only Python code I've written so far.
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
	
		f.write("void load_%d() {\n" % index)
		index += 1
		
		#used for materials
		f.write("\tfloat matColors[4];\n");
		
		#transformation data
		
		#first translate, then rotate, then scale
		# because they are applied from "back to front"
		
		#we also store the current matrix and restore it
		# after the object is rendered (push, pop)
		f.write("\tglPushMatrix();\n")
		
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
		
		f.write("\tglTranslated(%.6f, %.6f, %.6f);\n" % (locx, locy, locz))
		
		#rotation - this seems to be buggy !?
		if obj.RotX > 0.0001 or obj.RotX < -0.0001:
			f.write("  glRotated(%.6f,1,0,0);\n" % (-obj.RotX*180*0.31831))
		if obj.RotY > 0.0001 or obj.RotY < -0.0001:
			f.write("  glRotated(%.6f,0,1,0);\n" % (-obj.RotY*180*0.31831))
		if obj.RotZ > 0.0001 or obj.RotZ < -0.0001:
			f.write("  glRotated(%.6f,0,0,1);\n" % (-obj.RotZ*180*0.31831))
		
		#scaling
		f.write("\tglScaled(%.6f, %.6f, %.6f);\n" % tuple(obj.size))
		
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
					f.write("\tglMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,matColors);\n")
					
					#specular
					specs = material.getSpecCol()
					f.write("\tmatColors[0] = %.6f;\n" % specs[0])
					f.write("\tmatColors[1] = %.6f;\n" % specs[1])
					f.write("\tmatColors[2] = %.6f;\n" % specs[2])
					f.write("\tmatColors[3] = %.6f;\n" % material.getSpecTransp())
					f.write("\tglMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,matColors);\n")
			
			print "Exporting %s triangle(s)" % tricount  
			
			if len(face) == 3: f.write("\tglBegin(GL_TRIANGLES);\n")
			elif len(face) == 4: f.write("\tglBegin(GL_QUADS);\n")
			  
			for vertex in face.v:
				## Build glVertex3f
				f.write("\tglNormal3f(%.6f, %.6f, %.6f);\n" % tuple(vertex.no))
				f.write("\tglVertex3f(%.6f, %.6f, %.6f);\n" % tuple(vertex.co))
				vn+=1
			#print "glEnd();"
			f.write("\tglEnd();\n")
			
		f.write("\tglPopMatrix();\n")
		f.write("}\n")
	
	f.write("\n\n")
	
	#print "void loadModel() {"
	f.write("void loadModel() {\n")
	
	index = 0
	
	for obj in objects:
		f.write("\tload_%d();" % index)
		f.write("\t//object: %s\n" % obj.getData())
		index += 1
	
	f.write("}\n")
	
	print "Export complete"
	
	f.close()


Blender.Window.FileSelector(save_opengl, 'Export Wavefront OBJ', newFName('c'))