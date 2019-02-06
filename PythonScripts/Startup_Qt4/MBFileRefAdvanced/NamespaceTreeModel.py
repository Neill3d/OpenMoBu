from PySide import QtCore, QtGui

class node:
    def __init__( self, nodeData = '', parentNode = None ):
        self.childNodes = []
        self.nodeData = nodeData
        self.parentNode = parentNode
        
    def addChild( self, childNode ):
        if not childNode in self.childNodes:
            self.childNodes.append( childNode )
            childNode.parentNode = self

    def removeChild( self, childNode ):
        if childNode in self.childNodes:
            self.childNodes.remove( childNode )

    def removeAll( self ):
        self.childNodes = []

    def child( self, rowNum ):
        if rowNum < 0 or rowNum >= len(self.childNodes): return None
        return self.childNodes[rowNum]

    def indexOf( self, childNode ):
        if not childNode in self.childNodes: return -1
        return self.childNodes.index( childNode )

    def childCount( self ):
        return len( self.childNodes )

    def columnCount( self ):
        return 1

    def row( self ):
        if self.parentNode <> None:
            return self.parentNode.indexOf( self )
        return -1

    def data( self, column = 0 ):
        return self.nodeData

    def parent( self ):
        return self.parentNode

    def printNodes( self, level = 0 ):
        for index in range( 0, level ):
            print ' ',
        print self.nodeData
        for childNode in self.childNodes:
            childNode.printNodes( level + 1 )

class NamespaceTreeModel( QtCore.QAbstractItemModel ):
    def __init__( self, rootNode = node() ):
        super( QtCore.QAbstractItemModel, self ).__init__()

        self.rootNode = rootNode
        
    def index( self, rowNum, columnNum, parentIndex ):
        if not self.hasIndex( rowNum, columnNum, parentIndex ):
            return QtCore.QModelIndex()

        if not parentIndex.isValid():
            parentNode = self.rootNode
        else:
            parentNode = parentIndex.internalPointer()

        #print '::index(%d,%d,%s)' % ( rowNum, columnNum, parentNode.data() )

        lChildNode = parentNode.child( rowNum )
        if lChildNode <> None:
            #print '(%d,%d,%s)' % ( rowNum, columnNum, lChildNode.data() )
            return self.createIndex( rowNum, columnNum, lChildNode )
        else:
            return QtCore.QModelIndex()

    def parent( self, index ):
        #print '::parent(index(%d,%d,%s))' % ( index.row(), index.column(), index.internalPointer().data() )
        if not index.isValid():
            return QtCore.QModelIndex()
        
        curNode = index.internalPointer()
        if curNode == self.rootNode:
            return QtCore.QModelIndex()

        #print '**%s' % ( curNode.data() )
        return self.createIndex( curNode.row(), 0, curNode.parentNode )

    def rowCount( self, parentIndex ):
        #print '::rowCount(parentIndex(%d,%d))' % ( parentIndex.row(), parentIndex.column() )
        if parentIndex.column() > 0:
            return 0

        if not parentIndex.isValid():
            curNode = self.rootNode
        else:
            curNode = parentIndex.internalPointer()

        return curNode.childCount()

    def columnCount( self, parentIndex ):
        #print '::columnCount(parentIndex(%d,%d))' % ( parentIndex.row(), parentIndex.column() )
        if parentIndex.isValid():
            return parentIndex.internalPointer().columnCount()
        else:
            return self.rootNode.columnCount()

    def data( self, index, role ):
        #print '::data(index(%d,%d,%s), %s)' % ( index.row(), index.column(), index.internalPointer().data(), str(role) )
        if not index.isValid():
            return None

        if role <> QtCore.Qt.DisplayRole:
            return None

        return index.internalPointer().data( index.column() )

    def flags( self, index ):
        #print '::flags(index(%d,%d), %s)' % ( index.row(), index.column() )
        if not index.isValid():
            return 0

        return QtCore.Qt.ItemIsEnabled | QtCore.Qt.ItemIsSelectable

    def headerData( self, section, orientation, role ):
        #print '::headerData(%d, %s, %s)' % ( section, str(orientation), str(role) )
        if orientation == QtCore.Qt.Horizontal and role == QtCore.Qt.DisplayRole:
            return self.rootNode.data( section )

        return None