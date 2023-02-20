from os.path import expanduser
from PyQt6.QtWidgets import *
from PyQt6.QtGui import QStandardItemModel, QStandardItem
import re

log = []

with open('debug.txt') as f:
    for line in f:
        tag_match = re.search('\[.+?\]',line)
        if tag_match != None:
            tag = tag_match.group()[1:-1]
            content = line[tag_match.span()[1]:].replace('\n','')
            log.append({"tag": tag, "content": content})

app = QApplication([])
view = QTreeView()
#设置表头信息
model = QStandardItemModel()
model.setHorizontalHeaderLabels(['Tag', 'Content'])


itemProject = QStandardItem('项目')
model.appendRow(itemProject)
model.setItem(0, 1, QStandardItem('项目信息说明'))


parent = [itemProject]
for l in log:
    item = QStandardItem(l["tag"])
    parent[-1].appendRow(item)
    parent[-1].setChild(parent[-1].rowCount()-1,1,QStandardItem(l["content"]))
    if l["tag"]=="CallEnv":
        if re.search('enter',l['content'])!=None:
            print("parent")
            parent.append(item)
        elif re.search('exit',l['content'])!=None:
            parent.pop()

view.setModel(model)
view.show()
view.header().resizeSection(0, 160)
view.setStyle(QStyleFactory.create('windows'))
        #完全展开
# view.expandAll()

app.exec()