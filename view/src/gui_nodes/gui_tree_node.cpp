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

#include "include/gui_nodes/gui_tree_node.h"

namespace auto_trader {
namespace view {

GuiTreeNode::GuiTreeNode(const std::vector<QVariant> &data, int rowIndex, GuiTreeNodeType nodeType,
                         GuiTreeNode *parent)
    : data_(data), parentNode_(parent), nodeType_(nodeType), rowIndex_(rowIndex) {}

GuiTreeNode::~GuiTreeNode() { qDeleteAll(childNodes_); }

void GuiTreeNode::addNode(GuiTreeNode *node) { childNodes_.push_back(node); }

GuiTreeNode *GuiTreeNode::getChild(int index) {
  if (childNodes_.size() <= index) return nullptr;

  return childNodes_.at(index);
}

GuiTreeNode *GuiTreeNode::getParentNode() const { return parentNode_; }

size_t GuiTreeNode::getChildrenCounts() const { return childNodes_.size(); }

size_t GuiTreeNode::getColumnsCount() const { return data_.size(); }

int GuiTreeNode::getRowIndex() const { return rowIndex_; }

QVariant GuiTreeNode::data(int column) const { return data_.at(column); }

GuiTreeNodeType GuiTreeNode::getNodeType() const { return nodeType_; }

}  // namespace view
}  // namespace auto_trader