/*
 * Copyright (c) 2020, Rapprise.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "view/include/view_model/base_item_model.h"

#include "view/include/gui_nodes/gui_tree_node.h"

namespace auto_trader {
namespace view {

BaseItemModel::BaseItemModel(QObject *parent) : QAbstractItemModel(parent) {}

QVariant BaseItemModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }

  if (role != Qt::DisplayRole) {
    return QVariant();
  }

  GuiTreeNode *node = static_cast<GuiTreeNode *>(index.internalPointer());
  return node->data(index.column());
}

QVariant BaseItemModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    return rootNode_->data(section);
  }

  return QVariant();
}

Qt::ItemFlags BaseItemModel::flags(const QModelIndex &index) const {
  if (!index.isValid()) {
    return 0;
  }
  return QAbstractItemModel::flags(index);
}

QModelIndex BaseItemModel::index(int row, int column, const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  GuiTreeNode *parentNode;
  if (!parent.isValid()) {
    parentNode = rootNode_.get();
  } else {
    parentNode = static_cast<GuiTreeNode *>(parent.internalPointer());
  }

  GuiTreeNode *childItem = parentNode->getChild(row);
  if (childItem) {
    return createIndex(row, column, childItem);
  } else {
    return QModelIndex();
  }
}

QModelIndex BaseItemModel::parent(const QModelIndex &index) const {
  if (!index.isValid()) {
    return QModelIndex();
  }

  auto childNode = static_cast<GuiTreeNode *>(index.internalPointer());
  auto parentNode = childNode->getParentNode();

  if (parentNode == rootNode_.get()) {
    return QModelIndex();
  }

  return createIndex(parentNode->getRowIndex(), 0, parentNode);
}

int BaseItemModel::rowCount(const QModelIndex &parent) const {
  GuiTreeNode *parentNode;
  if (parent.column() > 0) {
    return 0;
  }

  if (!parent.isValid()) {
    parentNode = rootNode_.get();
  } else {
    parentNode = static_cast<GuiTreeNode *>(parent.internalPointer());
  }

  return parentNode->getChildrenCounts();
}

int BaseItemModel::columnCount(const QModelIndex &parent) const {
  if (parent.isValid()) {
    auto node = static_cast<GuiTreeNode *>(parent.internalPointer());
    return node->getColumnsCount();
  }

  return rootNode_->getColumnsCount();
}

GuiTreeNode *createGuiNode(const std::string &name, const std::string &value,
                           GuiTreeNodeType nodeType, GuiTreeNode *parent, int rowIndex) {
  std::vector<QVariant> content{QString::fromStdString(name), QString::fromStdString(value)};
  return new GuiTreeNode(content, rowIndex, nodeType, parent);
}

}  // namespace view
}  // namespace auto_trader