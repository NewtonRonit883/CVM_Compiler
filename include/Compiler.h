#pragma once
#include "Common.h"
#include "Parser.h"
#include <iomanip>

class Compiler {
    std::vector<uint8_t> code;
    std::unordered_map<std::string, uint8_t> vars;
    
    std::vector<std::vector<size_t>> breakStack;
    std::vector<std::vector<size_t>> continueStack; 
    
    uint8_t varCount = 0;
    void emit(uint8_t b) { code.push_back(b); }

    size_t emitJump(uint8_t op) {
        emit(op);
        emit(0); // placeholder
        return code.size() - 1;
    }

    void patchJump(size_t pos) {
        code[pos] = code.size();
    }

    void patchJumpTo(size_t pos, size_t target) {
        code[pos] = target;
    }

    void expr(ExprNode* e) {
        if (!e) return;

        if (auto* n = dynamic_cast<IntNode*>(e)) {
            emit(OP_PUSH); emit(n->value);
        }
        else if (auto* v = dynamic_cast<VarNode*>(e)) {
            emit(OP_LOAD); emit(vars[std::string(v->name)]);
        }
        else if (auto* a = dynamic_cast<AddNode*>(e)) {
            expr(a->l); expr(a->r); emit(OP_ADD);
        }
        else if (auto* s = dynamic_cast<SubNode*>(e)) {
            expr(s->l); expr(s->r); emit(OP_SUB);
        }
        else if (auto* m = dynamic_cast<MulNode*>(e)) {
            expr(m->l); expr(m->r); emit(OP_MUL);
        }
        else if (auto* d = dynamic_cast<DivNode*>(e)) {
            expr(d->l); expr(d->r); emit(OP_DIV);
        }
        else if (auto* m = dynamic_cast<ModNode*>(e)) { // <-- Add this block
            expr(m->l); expr(m->r); emit(OP_MOD);
        }
        else if (auto* l = dynamic_cast<LessNode*>(e)) {           
            expr(l->l); expr(l->r); emit(OP_LESS);         
        }
        else if (auto* g = dynamic_cast<GreaterNode*>(e)) {            
            expr(g->l); expr(g->r); emit(OP_GREATER);      
        }
        else if (auto* eq = dynamic_cast<EqualNode*>(e)) {
            expr(eq->l); expr(eq->r); emit(OP_EQUAL);
        }
    }

    void stmt(StmtNode* s) {
        if (!s) return;

        if (auto* l = dynamic_cast<LetNode*>(s)) {
            expr(l->expr);
            std::string nameStr = std::string(l->name);
            if (!vars.count(nameStr)) vars[nameStr] = varCount++;
            emit(OP_STORE); emit(vars[nameStr]);
        }

        else if (auto* p = dynamic_cast<PrintNode*>(s)) {
            expr(p->expr);
            emit(OP_PRINT);
        }

        else if (auto* i = dynamic_cast<IfNode*>(s)) {
            expr(i->cond);

            size_t jFalse = emitJump(OP_JUMP_IF_FALSE);

            for (auto* st : i->thenBlock->stmts)
                stmt(st);

            size_t jEnd = emitJump(OP_JUMP);

            patchJump(jFalse);

            if (i->elseBlock) {
                for (auto* st : i->elseBlock->stmts)
                    stmt(st);
            }

            patchJump(jEnd);
        }
        
        else if (auto* w = dynamic_cast<WhileNode*>(s)) {
            size_t loopStart = code.size();
            
            breakStack.push_back({});
            continueStack.push_back({}); 

            expr(w->cond);

            size_t jFalse = emitJump(OP_JUMP_IF_FALSE);

            for (auto* st : w->body->stmts)
                stmt(st);

            emit(OP_JUMP);
            emit(loopStart);

            patchJump(jFalse);

            for (auto pos : breakStack.back()) patchJump(pos);
            for (auto pos : continueStack.back()) patchJumpTo(pos, loopStart);

            breakStack.pop_back();
            continueStack.pop_back();
        }

        else if (auto* d = dynamic_cast<DoWhileNode*>(s)) {
            size_t loopStart = code.size();
            
            breakStack.push_back({});
            continueStack.push_back({});

            for (auto* st : d->body->stmts)
                stmt(st);

            size_t condStart = code.size();

            expr(d->cond);
            
            size_t jFalse = emitJump(OP_JUMP_IF_FALSE);
            emit(OP_JUMP);
            emit(loopStart);
            
            patchJump(jFalse);

            for (auto pos : breakStack.back()) patchJump(pos);
            for (auto pos : continueStack.back()) patchJumpTo(pos, condStart);

            breakStack.pop_back();
            continueStack.pop_back();
        }

        else if (auto* f = dynamic_cast<ForNode*>(s)) {
            breakStack.push_back({});
            continueStack.push_back({});

            if (f->init) stmt(f->init);

            size_t loopStart = code.size();
            
            size_t jFalse = -1;
            if (f->cond) {
                expr(f->cond);
                jFalse = emitJump(OP_JUMP_IF_FALSE);
            }

            for (auto* st : f->body->stmts)
                stmt(st);

            size_t incStart = code.size();
            if (f->inc) stmt(f->inc);

            emit(OP_JUMP);
            emit(loopStart);

            if (jFalse != -1) patchJump(jFalse);

            for (auto pos : breakStack.back()) patchJump(pos);
            for (auto pos : continueStack.back()) patchJumpTo(pos, incStart);

            breakStack.pop_back();
            continueStack.pop_back();
        }

        else if (dynamic_cast<BreakNode*>(s)) {
            size_t j = emitJump(OP_JUMP);
            breakStack.back().push_back(j);
        }
        else if (dynamic_cast<ContinueNode*>(s)) {
            size_t j = emitJump(OP_JUMP);
            continueStack.back().push_back(j); 
        }
    }

public:
    void dumpBytecode() const {
        std::cout << "\n--- Generated Bytecode Blueprint ---\n";
        for (size_t i = 0; i < code.size(); ) {
            std::cout << "  Offset " << std::setw(3) << std::setfill('0') << i << ": ";
            uint8_t op = code[i];
            std::cout << std::left << std::setw(18) << opcodeToString(op);
            if (op == OP_PUSH || op == OP_STORE || op == OP_LOAD || op == OP_JUMP || op == OP_JUMP_IF_FALSE) {
                if (i + 1 < code.size()) {
                    std::cout << " [Operand: " << (int)code[i + 1] << "]";
                }
                i += 2;
            } else {
                i += 1;
            }
            std::cout << "\n";
        }
        std::cout << "------------------------------------\n";
    }

    std::vector<uint8_t> compile(const std::vector<StmtNode*>& ast) {
        code.clear();       // <-- ADD THIS LINE HERE
        code.reserve(1024); 
        for (auto* s : ast) stmt(s);
        emit(OP_HALT);
        
        dumpBytecode(); 
        return code;
    }
};
