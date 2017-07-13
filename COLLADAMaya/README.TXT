COLLADAMaya is a new implementation of a Autodesk Maya plug-in to export a 
Maya scene or parts of it to a COLLADA file, released under the MIT-license. 

This plug-in does not store the COLLADA document in an intermidiate data 
model, it writes the data directly into file. This results in reduction of 
memory consumption and better performance.

For installation instructions please read the "INSTALL" file.


- General export options in OpenColladaMaya export plugin:

	. Bake transforms
Enabled:	Transforms are exported as matrices. Animations and controllers are baked.
Disabled:	Transforms are exported as transform stacks (<translate>/<rotate>/<scale>...). Animations are exported as Bezier curves. Controllers are not exported.

You can export animations using two different modes: Bezier or baked.
In Bezier mode, no sampling is done to get the value of animation keys.
In Baked mode, sampling is performed to get animation value at constant sampling rate.
Use "Bake transforms" option if you want to bake animations. In the other cases you will have bezier animation exported
	
	. Relative paths
Enabled:	Exported paths are relative to COLLADA file.
Disabled:	Exported paths are absolute.
	
	. Copy textures
Enabled:	Copy referenced textures to destination directory. Paths to textures in COLLADA document point to copied textures.
Disabled:	Texture are not copied. Paths to textures in COLLADA document point to original texture files.
	
	. Preserve source tree
Enabled:	Source directory structure is preserved. Example: export scene.ma to folder_b:

Sources:
folder_a
|_ scene.ma
|_ textures
   |_ wood
   |  |_ wood_a.jpg
   |  |_ wood_b.jpg
   |_ rock
      |_ rock_a.jpg
	  |_ rock_b.jpg
	  
Export destination:
folder_b
|_ scene.dae
|_ textures
   |_ wood
   |  |_ wood_a.jpg
   |  |_ wood_b.jpg
   |_ rock
      |_ rock_a.jpg
	  |_ rock_b.jpg

Disabled:	All tetures are copied to COLLADA document destination folder. With previous example:
Export destination:
folder_b
|_ scene.dae
|_ wood_a.jpg
|_ wood_b.jpg
|_ rock_a.jpg
|_ rock_b.jpg
	
	. Triangulate
Enabled:	Polygons with more than 3 edges are triangulated.
Disabled:	Polygons are not triangulated.
	
	. Cgfx file references
Enabled:	File references to cgfx files are exported.
Disabled:	Content of cgfx file is exported into COLLADA document.
	
	. Sampling
	
	. Curve constrain
	
	. Static curve removal

	. Encode names
By default, All names (https://www.w3.org/TR/REC-xml/#NT-Name) will be UCS-2 encoded (https://msdn.microsoft.com/en-us/library/35577sxd(v=vs.80).aspx)
Any XML name character that does not conform to the XML 1.0 spec (fourth edition) recommendation is escaped as _xHHHH_
The HHHH string stands for the four-digit hexadecimal UCS-2 code for the character in most significant bit first order
For example, the name "Order Details" is encoded as "Order_x0020_Details".

If you need backward compatible behavior (all non conformant characters will be replaced by '_' ) please turn off "Encode Names" option

	. Polygon meshes
Export geometries.
	
	. Lights
Export lights.
	
	. Cameras
Export cameras.
	
	. Joints
Export joints.
	
	. Skin
Export skins.
	
	. Animations
Export animations.
	
	. Optimize animations
Don't export bezier animations that are constant.

If you export animations with "Bake transforms" option enabled, constant animations (same value for all keyframes) will be automatically removed.
If you want to remove constant key frames in Bezier mode, use "Optimize Animations" option.
When you export Bezier animation, you will have animation value for translation X,Y,Z and Rotation X,Y,Z even if the value is constant.
When you use "Optimize Animations" in Bezier Animation mode, only useful (not constant value) from translation X,Y,Z or Rotation X,Y,Z will be exported.
	
	. Convex mesh geometries
Enabled:	Convex meshes actual geometry is exported to COLLADA document.
Disabled:	Convex meshes geometry is not exported to COLLADA document. Loaders have to compute convex geometry themselves.
	
	. Physics
Export Physics. (Requires nVidia PhysX plugin. See below)
	
	. Invisible nodes
Export invisible nodes.
	
	. Texture coordinates
Export texture coordinates.
	
	. Normals
Export normals.
	
	. Normals per vertex
Export normals per vertex or per face.
	
	. Vertex colors
Export vertex color.
	
	. Vertex colors per vertex
Export vertex color per vertex or per face.
	
	. Texture tangents
Export texture tangents.
	
	. Geometric tangents
Export geometric tangents. Not exported if normals are not exported.
	
	. Referenced materials only
Enabled:	Export only materials that are reference in the Maya scene.
Disabled:	Export all materials from Maya material library.
	
	. Materials only
Export only materials to COLLADA document. (No geometry, no node, only materials)
	
	. Export to separate files
Allows to split animation/mesh into two differents DAE files:
Use Model option (equivalent to Polygon Meshes + Skin + Joints options) if you want to export only Mesh into one DAE.
Use Anim option(equivalent to Animations + Joints options if you want to export only Animation into one DAE.
If you want to export full animation and mesh into the same DAE, use Polygon Meshes + Skin + Joints + Animations options.
	
	. Export references
Export Maya references (as COLLADA external references).
	
	. Dereference
When exporting references: embed references into COLLADA documents (no COLLADA external references).
	
	. Export camera as lookat
Export camera node transform as COLLADA <lookat>.
	
	. XFov
Export camera X FOV. If both XFov and YFov are disabled then aspect ratio is exported.

	. YFov
Export camera Y FOV. If YFov is disabled then  XFov is exported. If both XFov and YFov are disabled then aspect ratio is exported.
	
	. Export double precision
Floating point value are exported with double precision.


- Pose Animation:
	
If you want to export Pose Animation (only 1 key frame animation), be careful to set Source Start = Source End = 0 in Animation Clip Attributes in Maya


- Extra Attributes

This plug-in currently supports export of Maya extra attributes for the following Maya node types:
	. Transform (in <node>)
	. Mesh shape (in <geometry>)
	. Light shape (in <light>)
	. PhysX RigidBody (in <rigid_body>)
	. PhysX PhysicsShape (in <shape>)
	. PhysX RigidConstraint (in <rigid_constraint>)

Extra attributes are exported in <extra>/<technique profile="OpenCOLLADAMaya"> section of corresponding COLLADA element with the following syntax:

<param name="attribute_name" type="attribute_type">attribute_value</param>

	. attribute_name is the name of the extra attribute.
	. attribute_type is the type of the extra attribute. Supported types are:
		. "int" (Maya extra attribute type: Integer)
		. "string" (Maya extra attribute type: String or Enum)
		. "float" (Maya extra attribute type: Float)
		. "bool" (Maya extra attribute type: Boolean)
		. "double3" (Maya extra attribute type: Vector)
	. attribute_value is the extra attribute value as text.

- Extensions: 
OpenCOLLADAMaya plugin supports LOD feature using Instance node extension
https://www.khronos.org/collada/wiki/Instance_node_OpenCOLLADA_extension
OpenCOLLADAMaya plugin supports AnimationClip Event using animationClip extension
https://www.khronos.org/collada/wiki/Animation_clip_OpenCOLLADA_extension
	
	
- Physics

OpenCOLLADAMaya plugin currently supports export of PhysX scenes created with nVidia PhysX plugin. PhysX scene data is exported using several extensions described here:

https://www.khronos.org/collada/wiki/Physics_material_OpenCOLLADA_extension
https://www.khronos.org/collada/wiki/Rigid_body_OpenCOLLADA_extension
https://www.khronos.org/collada/wiki/Rigid_constraint_OpenCOLLADA_extension
https://www.khronos.org/collada/wiki/Shape_OpenCOLLADA_extension

An additional parameter is exported in OpenCOLLADAMaya profile for <rigid_body> element:
<force_to_sleep>: boolean. True if rigid body is initially sleeping when physics simulation starts. False if rigid body is awake when physics simulation starts.