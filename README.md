# UnityFbx
Converts FBX files into a format that can be loaded at runtime in Unity

Usage:
pwimport <fbx file> [options]
-o <directory> (Change directory for converted mesh, collision, and material files)
-t <directory> (Change directory for converted textures)
-c (Export as a collision model)
-n (Do not import normals)

Example: pwimport "C:\FbxFiles\MyModel.fbx" -o "C:\Protowave\Models" -t "C:\Protowave\Materials"
