import bpy

class MYADDON_OT_create_ico_sphere(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_create_ico_sphere"
    bl_label = "ICO球生成"
    bl_description = "ICO球を生成します"

    #Redo,Undo可能オプション
    bl_option = {'REGISTER' , 'UNDO'}

    def execute(self,context):
        bpy.ops.mesh.primitive_ico_sphere_add()
        print("ICO球を生成しました")

        return {'FINISHED'} 
    