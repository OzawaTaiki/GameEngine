import bpy
import gpu
import gpu_extras.batch
import copy
import mathutils



class DrawCollider:

    #描画ハンドル
    handler = None

    def draw_collider():
        #頂点データ
        vertices ={"pos":[]}
        #インデックスデータ
        indices = []

        offsets=[
            [-0.5,-0.5,-0.5],#左下前
            [+0.5,-0.5,-0.5],#右下前
            [-0.5,+0.5,-0.5],#左上前
            [+0.5,+0.5,-0.5],#右上前
            [-0.5,-0.5,+0.5],#左下奥
            [+0.5,-0.5,+0.5],#右下奥
            [-0.5,+0.5,+0.5],#左上奥
            [+0.5,+0.5,+0.5]#右上奥            
        ]
        size = [2,2,2]

        for object in bpy.context.scene.objects:

            #コライダープロパティがなければスキップ
            if "collider" not in object:
                continue

            center = mathutils.Vector((0,0,0))
            size = mathutils.Vector((2,2,2))

            #プロパティからコライダーの値を取得
            center[0]= object["collider_center"][0]
            center[1]= object["collider_center"][1]
            center[2]= object["collider_center"][2]
            size[0]= object["collider_size"][0]
            size[1]= object["collider_size"][1]
            size[2]= object["collider_size"][2]

            #追加前の頂点数
            start = len(vertices["pos"])

            #boxの8頂点分回す
            for offset in offsets:
                #オブジェクトの中心座標をコピー
                pos = copy.copy(center)
                #中心座標を基準に各頂点ごとにずらす
                pos[0] += offset[0] * size[0]
                pos[1] += offset[1] * size[1]
                pos[2] += offset[2] * size[2]

                #オブジェクトのローカル座標をワールド座標に変換
                pos = object.matrix_world @ pos

                #頂点データに追加
                vertices["pos"].append(pos)

                indices.append([start+0,start+1])
                indices.append([start+2,start+3])
                indices.append([start+0,start+2])
                indices.append([start+1,start+3])

                indices.append([start+4,start+5])
                indices.append([start+6,start+7])
                indices.append([start+4,start+6])
                indices.append([start+5,start+7])

                indices.append([start+0,start+4])
                indices.append([start+1,start+5])
                indices.append([start+2,start+6])
                indices.append([start+3,start+7])


        #ビルトインのシェーダーを使用
        shader = gpu.shader.from_builtin("UNIFORM_COLOR")
        #バッチを作成
        batch = gpu_extras.batch.batch_for_shader(shader, 'LINES', vertices, indices=indices)

        #シェーダーのパラメータを設定
        color = {0.5, 1.0, 1.0, 1.0}
        shader.bind()
        shader.uniform_float("color", color)
        #バッチを描画
        batch.draw(shader)


class MYADDON_OT_add_collider(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_add_collider"
    bl_label = "Collider 追加"
    bl_description = "[Collider]カスタムプロパティを追加します"

    #Redo,Undo可能オプション
    bl_option = {'REGISTER' , 'UNDO'}

    def execute(self,context):
        
        #['Collider']カスタムプロパティを追加
        context.object["collider"] = "BOX"
        context.object["collider_center"] = mathutils.Vector((0,0,0))
        context.object["collider_size"] = mathutils.Vector((2,2,2))

        return {'FINISHED'}

class OBJECT_PT_collider(bpy.types.Panel):
    """ オブジェクトのColliderパネル """
    bl_idname = "OBJECT_PT_collider"
    bl_label = "Collider"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    #サブメニューの描画
    def draw(self,context):

        if "collider" in context.object:
            #カスタムプロパティが存在する場合は表示
            self.layout.prop(context.object, '["collider"]', text = self.bl_label)
            self.layout.prop(context.object, '["collider_center"]', text = "Center")
            self.layout.prop(context.object, '["collider_size"]', text = "Size")

        else:
            #カスタムプロパティが存在しない場合は追加ボタンを表示
            self.layout.operator(MYADDON_OT_add_collider.bl_idname)

        #パネルの項目を追加
        #self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text=MYADDON_OT_stretch_vertex.bl_label)
        #self.layout.operator(MYADDON_OT_create_ico_sphere.bl_idname, text=MYADDON_OT_create_ico_sphere.bl_label)
        #self.layout.operator(MYADDON_OT_export_scene.bl_idname, text=MYADDON_OT_export_scene.bl_label)
