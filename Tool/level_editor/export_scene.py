import bpy
import math
import bpy_extras
import json

class MYADDON_OT_export_scene(bpy.types.Operator,bpy_extras.io_utils.ExportHelper):
    bl_idname = "myaddon.myaddon_ot_export_scene"
    bl_label = "シーン出力"
    bl_description = "シーン情報をexportします"
    #出力するファイルの拡張子
    filename_ext=".json"

    def execute(self,context):
        print("シーン情報を出力します")
        print(bpy.context.scene.objects)
        
        self.export_json()

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

    def export_json(self):
        """シーン情報をjson形式で書き出し"""

        #保存する情報をまとめるdict
        json_object_root = dict()

        #ノード名
        json_object_root["name"] = "scene"
        #オブジェクトリストを作成
        json_object_root["objects"] = list()

        #シーン内の全てのオブジェクトを走査
        for object in bpy.context.scene.objects:
            #親オブジェクトはスキップ
            if(object.parent):
                continue
            
            #シーン直下のオブジェクトをルートノードとして再帰で走査
            self.parse_scene_recursive_json(json_object_root["objects"],object,0)
            


        json_text = json.dumps(json_object_root,ensure_ascii=False,cls=json.JSONEncoder,indent =4)
        #コンソールに表示
        print(json_text)

        #ファイルをテキスト形式で書き出し用にオープン
        #スコープを抜けると自動でクローズされる
        with open(self.filepath,"wt",encoding="utf-8") as file:
            #ファイルに文字列を書き込む
            file.write(json_text)
            

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
        #座標系を変換して出力する
        self.write_and_print(file, indent + "T %f %f %f" % (trans.x, trans.z, trans. y) )
        self.write_and_print(file, indent + "R %f %f %f" % (-rot.x, -rot.z, -rot.y) )
        self.write_and_print(file, indent + "S %f %f %f" % (scale.x, scale.z, scale.y) )

        #カスタムプロパティの書き込み
        if "file_name" in object:
            self.write_and_print(file,indent + "N %s" % object["file_name"])
        
        # Colliderの書き出し
        if "collider" in object:
            self.write_and_print(file,indent + "C %s" % object["collider"])
            temp_str = indent + "CC %f %f %f"
            temp_str %= (object["collider_center"][0], object["collider_center"][2], object["collider_center"][1])
            self.write_and_print(file,temp_str)
            temp_str = indent + "CS %f %f %f"
            temp_str %= (object["collider_size"][0], object["collider_size"][2], object["collider_size"][1])
            self.write_and_print(file,temp_str)

        self.write_and_print(file,indent + "END")
        self.write_and_print(file, '')

        for child in object.children:
            self.parse_scene_recursive(file ,child ,level + 1)


    def parse_scene_recursive_json(self, data_parent, object, level):
        #シーンのオブジェクト一個分のjsonデータを作成
        json_object = dict()
        #objectの種類
        json_object["type"] = object.type
        #オブジェクト名
        json_object["name"] = object.name

        #オブジェクトのローカルトランスフォームをから
        #平行移動 回転 スケールを取得
        trans,rot,scale = object.matrix_local.decompose()
        #回転を Quternion から Euler (3軸での回転角)に変換
        rot = rot.to_euler()
        
        #ラジアンから度数法に変換
        #rot.x = math.degrees(rot.x)
        #rot.y = math.degrees(rot.y)
        #rot.z = math.degrees(rot.z)

        #トランスフォーム情報をディクショナリに登録
        transform = dict()
        transform["transform"] = (trans.x,trans.z,trans.y)
        transform["rotation"] = (-rot.x,-rot.z,-rot.y)
        transform["scale"] = (scale.x,scale.z,scale.y)

        #トランスフォーム情報をjsonオブジェクトに登録
        json_object["transform"] = transform

        #カスタムプロパティの書き込み
        if "file_name" in object:
            #カスタムプロパティが存在する場合は登録
            json_object["file_name"] = object["file_name"]

        if "collider" in object:
            #カスタムプロパティが存在する場合は登録
            collider = dict()
            collider["type"] = object["collider"]
            collider["center"] = object["collider_center"].to_list()
            collider["size"] = object["collider_size"].to_list()
            json_object["collider"] = collider
    


        #一個分のjsonオブジェクトを親オブジェクトに登録
        data_parent.append(json_object)

        #直接子供のリストを走査
        if len(object.children) > 0:
            #子ノードリストを作成
            json_object["children"] = list()

            #子供のリストを走査
            for child in object.children:
                #子供のjsonデータを作成
                self.parse_scene_recursive_json(json_object["children"],child,level + 1)

