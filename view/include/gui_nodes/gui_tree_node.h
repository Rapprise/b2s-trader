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

#ifndef AUTO_TRADER_VIEW_GUI_TREE_NODE_H
#define AUTO_TRADER_VIEW_GUI_TREE_NODE_H

#include <QtCore/QList>
#include <QtCore/QVariant>

#include "gui_tree_node_type.h"

namespace auto_trader {
namespace view {

class GuiTreeNode {
 public:
  GuiTreeNode(const std::vector<QVariant> &data, int rowIndex, GuiTreeNodeType nodeType,
              GuiTreeNode *parent = 0);
  ~GuiTreeNode();

  void addNode(GuiTreeNode *node);
  GuiTreeNode *getChild(int index);
  GuiTreeNode *getParentNode() const;
  GuiTreeNodeType getNodeType() const;

  size_t getChildrenCounts() const;
  size_t getColumnsCount() const;

  int getRowIndex() const;

  QVariant data(int column) const;

 private:
  std::vector<GuiTreeNode *> childNodes_;
  std::vector<QVariant> data_;

  GuiTreeNode *parentNode_;
  GuiTreeNodeType nodeType_;

  int rowIndex_;
};

}  // namespace view
}  // namespace auto_trader

#endif  // AUTO_TRADER_VIEW_GUI_TREE_NODE_H
