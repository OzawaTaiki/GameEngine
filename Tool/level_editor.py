import bpy
import math
import bpy_extras
import gpu
import gpu_extras.batch

bl_info = {
    "name": "レベルエディタ",
    "author": "Taiki Ozawa",
    "version": (1 ,0),
    "blender": (3 ,3 ,1),
    "location": "",
    "description": "レベルエディタ",
    "warning": "",		
    "wiki_url": "",
    "tracker_url": "",
    "category": "Object"
}

class MYADDON_OT_stretch_vertex(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_t_stretch_vertex"
    bl_label = "頂点を伸ばす"
    bl_description = "頂点座標を引っ張ってのばす"

    #Redo,Undo可能オプション
    bl_option = {'REGISTER' , 'UNDO'}

    def execute(self,context):
        bpy.data.objects["Cube"].data.vertices[0].co.x += 1.0
        print("頂点をのばしました")

        return {'FINISHED'}


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
    
class MYADDON_OT_export_scene(bpy.types.Operator,bpy_extras.io_utils.ExportHelper):
    bl_idname = "myaddon.myaddon_ot_export_scene"
    bl_label = "シーン出力"
    bl_description = "シーン情報をexportします"
    #出力するファイルの拡張子
    filename_ext=".scene"

    def execute(self,context):
        print("シーン情報を出力します")
        print(bpy.context.scene.objects)
        
        self.export()

        print("シーン情報を出力しました")
        self.report({'INFO'},"シーン情報を出力しました")

        return {'FINISHED'}

    def export(self):

        print("シーン情報を出力開始．．．%r" % self.filepath)
        
        #ファイルをテキスト形式で書き出しようにオープン
        #スコープを抜けると自動でクローズされる
        with open(self.filepath,"wt") as file:
            #ファイルに文字列を書き込む
            file.write("SCENE\n")

            for object in bpy.context.scene.objects:
                if(object.parent):
                    continue
            
                self.parse_scene_recursive(file,object,0)
                                     

    def write_and_print(self,file,str):
        print(str)

        file.write(str)
        file.write("\n")

    def parse_scene_recursive(self, file, object, level):
        """シーン解析用再帰関数"""

        indent = ''
        for i in range(level):
            indent += "\t"

        # オブジェクト名書き込み
        self.write_and_print(file,indent + object.type )
        trans, rot, scale = object.matrix_local. decompose( )
        #回転を Quternion から Euler (3軸での回転角)に変換
        rot = rot. to_euler( )
        #ラジアンから度数法に変換
        rot.x = math.degrees(rot.x)
        rot.y = math.degrees(rot.y)
        rot.z = math.degrees(rot.z)
        #トランスフォーム情報を表示
        self.write_and_print(file, indent + "T %f %f %f" % (trans.x, trans.y, trans. z) )
        self.write_and_print(file, indent + "R %f %f %f" % (rot.x, rot.y, rot. z) )
        self.write_and_print(file, indent + "S %f %f %f" % (scale.x, scale.y, scale.z) )

        #カスタムプロパティの書き込み
        if "file_name" in object:
            self.write_and_print(file,indent + "N %s" % object["file_name"])
        
        self.write_and_print(file,indent + "END")
        self.write_and_print(file, '')

        for child in object.children:
            self.parse_scene_recursive(file ,child ,level + 1)


class MYADDON_OT_add_filename(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_add_filename"
    bl_label = "FileName 追加"
    bl_description = "['file_name']カスタムプロパティを追加します"
    bl_option = {'REGISTER' , 'UNDO'}

    def execute(self,context):
        #['file_name']カスタムプロパティを追加
        context.object["file_name"] = ""

        return {'FINISHED'}            
    

#トップバーの拡張メニュー
class TOPBAR_MT_my_menu(bpy.types.Menu):
    #Blenderがクラスを識別するための固有の文字列
    bl_idname= "TOPBAR_MT_my_menu"
    #メニューのラベルとして表示される文字列
    bl_label = "MyMenu"
    #著者表示用の文字列
    bl_description = "拡張メニュー by" + bl_info["author"]

    def draw(self,context):
        #self : 呼び出し元のクラスインスタンス c++のthisポインタ
        #contet : カーソルを合わせたときのポップアップのカスタマイズなどに使用

        #self.layout.operator("wm.url_open_preset",text = "Manual", icon ='HELP')
        #self.layout.separator()
        #self.layout.operator("wm.url_open_preset",text = "Manual", icon ='HELP')
        self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname,text=MYADDON_OT_stretch_vertex.bl_label)
        self.layout.operator(MYADDON_OT_create_ico_sphere.bl_idname,text=MYADDON_OT_create_ico_sphere.bl_label)
        self.layout.operator(MYADDON_OT_export_scene.bl_idname,text=MYADDON_OT_export_scene.bl_label)


    def submenu(self,context):
        #self : 呼び出し元のクラスインスタンス c++のthisポインタ
        #contet : カーソルを合わせたときのポップアップのカスタマイズなどに使用

        self.layout.menu(TOPBAR_MT_my_menu.bl_idname)


class OBJECT_PT_file_name(bpy.types.Panel):
    """ オブジェクトのファイルネームパネル """
    bl_idname = "OBJECT_PT_file_name"
    bl_label = "FIleName"
    bl_space_type = "PROPERTIES"
    bl_region_type = "WINDOW"
    bl_context = "object"

    #サブメニューの描画
    def draw(self,context):

        if "file_name" in context.object:
            #カスタムプロパティが存在する場合は表示
            self.layout.prop(context.object, '["file_name"]', text = self.bl_label)
        else:
            #カスタムプロパティが存在しない場合は追加ボタンを表示
            self.layout.operator(MYADDON_OT_add_filename.bl_idname)

        #パネルの項目を追加
        #self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text=MYADDON_OT_stretch_vertex.bl_label)
        #self.layout.operator(MYADDON_OT_create_ico_sphere.bl_idname, text=MYADDON_OT_create_ico_sphere.bl_label)
        #self.layout.operator(MYADDON_OT_export_scene.bl_idname, text=MYADDON_OT_export_scene.bl_label)


class DrawCollider:

    #描画ハンドル
    handler = None

    def draw_collider():
        #頂点データ
        vertices ={"pos":[[0,0,0],[2,2,2]]}
        #インデックスデータ
        indices = [0,1]

        #ビルトインのシェーダーを使用
        shader = gpu.shader.from_builtin('3D_UNIFORM_COLOR')
        #バッチを作成
        batch = gpu_extras.batch.batch_for_shader(shader, 'LINES', vertices,indices = indices)

        #シェーダーのパラメータを設定
        color = {0.5, 1.0, 1.0, 1.0}
        shader.bind()
        shader.uniform_float("color", color)
        #バッチを描画
        batch.draw(shader)

#TODO: 01_09 p8まで

classes = {
    MYADDON_OT_stretch_vertex,
    MYADDON_OT_create_ico_sphere,
    MYADDON_OT_export_scene,
    MYADDON_OT_add_filename,
    TOPBAR_MT_my_menu,
    OBJECT_PT_file_name,
}


#アドオン有効化時のコールバック
def register():
    for cls in classes:
        bpy.utils.register_class(cls)

    bpy.types.TOPBAR_MT_editor_menus.append(TOPBAR_MT_my_menu.submenu)
    print("レベルエディタが有効化されました")

#アドオン無効化時のコールバック
def unregister():
    bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_my_menu.submenu)

    for cls in classes:
        bpy.utils.unregister_class(cls)
        
    print("レベルエディタが無効化されました。")



#テスト実行用コード
if __name__ == "__main__":
    register()