import bpy
import mathutils

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

from .stretch_vertex import MYADDON_OT_stretch_vertex
from .create_ico_sphere import MYADDON_OT_create_ico_sphere
from .export_scene import MYADDON_OT_export_scene
from .collider_system import MYADDON_OT_add_collider, OBJECT_PT_collider, DrawCollider
    
class MYADDON_OT_add_filename(bpy.types.Operator):
    bl_idname = "myaddon.myaddon_add_filename"
    bl_label = "FileName 追加"
    bl_description = "['file_name']カスタムプロパティを追加します"
    bl_options = {'REGISTER' , 'UNDO'}

    def execute(self,context):
        #['file_name']カスタムプロパティを追加
        context.object["file_name"] = ""

        return {'FINISHED'}            
    

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


classes = {
    MYADDON_OT_stretch_vertex,
    MYADDON_OT_create_ico_sphere,
    MYADDON_OT_export_scene,
    MYADDON_OT_add_filename,
    TOPBAR_MT_my_menu,
    OBJECT_PT_file_name,
    MYADDON_OT_add_collider,
    OBJECT_PT_collider,
}


#アドオン有効化時のコールバック
def register():
    for cls in classes:
        bpy.utils.register_class(cls)

    bpy.types.TOPBAR_MT_editor_menus.append(TOPBAR_MT_my_menu.submenu)
    # 3Dビューに描画関数を追加
    DrawCollider.handler = bpy.types.SpaceView3D.draw_handler_add(DrawCollider.draw_collider, (), 'WINDOW', 'POST_VIEW')
    print("レベルエディタが有効化されました")

#アドオン無効化時のコールバック
def unregister():
    bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_my_menu.submenu)
    # 3Dビューから描画関数を削除
    bpy.types.SpaceView3D.draw_handler_remove(DrawCollider.handler, 'WINDOW')

    for cls in classes:
        bpy.utils.unregister_class(cls)
        
    print("レベルエディタが無効化されました。")



#テスト実行用コード
if __name__ == "__main__":
    register()