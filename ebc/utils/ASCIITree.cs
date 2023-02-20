public class Node{
    public Object value = null;
    public IEnumerable<Node> childs = new List<Node>();
    public override string ToString(){
        return Print("\n",true);
    }
    private string Print(string indent, bool isLast){
        string ret = indent;
        if(isLast){
            ret += "  └─";
            indent += "    ";
        }
        else{
            ret += "  ├─";
            indent += "  │ ";
        }
        
        if(value!=null){
            var content = value.ToString();
            if(content.Contains('\n')){
                var tmp = '╖' + content.Replace("\n",indent + "║");
                var index = tmp.LastIndexOf('║');
                ret += tmp.Remove(index,1).Insert(index,"╚");
            }
            else{
                ret += content;
            }
        }

        if(childs != null){
            int i = 0;
            foreach(var child in childs){
                ret += child.Print(indent,i==childs.Count()-1);
                i++;
            }
        }

        return ret;
    }
}

