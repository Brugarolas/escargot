/*
 * Copyright (c) 2020-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#ifndef __EscargotAsyncGeneratorObject__
#define __EscargotAsyncGeneratorObject__

#include "runtime/Object.h"
#include "runtime/ScriptFunctionObject.h"
#include "runtime/ExecutionPauser.h"
#include "runtime/PromiseObject.h"
#include "interpreter/ByteCode.h"

namespace Escargot {

class ScriptAsyncGeneratorFunctionHelperFunctionObject;

class AsyncGeneratorObject : public Object {
    friend class ByteCodeInterpreter;
    friend class ExecutionPauser;

public:
    enum AsyncGeneratorEnqueueType {
        Next,
        Return,
        Throw
    };

    enum AsyncGeneratorState {
        Executing,
        Completed,
        AwaitingReturn,
        SuspendedStart,
        SuspendedYield
    };

    AsyncGeneratorObject(ExecutionState& state);
    AsyncGeneratorObject(ExecutionState& state, ExecutionState* executionState, Value* registerFile, ByteCodeBlock* blk, const Value& prototype);

    virtual const char* internalClassProperty() override
    {
        return "AsyncGenerator";
    }

    virtual bool isAsyncGeneratorObject() const override
    {
        return true;
    }

    void* operator new(size_t size);
    void* operator new[](size_t size) = delete;

    AsyncGeneratorState asyncGeneratorState() const
    {
        return m_asyncGeneratorState;
    }

    ExecutionPauser* executionPauser()
    {
        return &m_executionPauser;
    }

private:
    static inline void fillGCDescriptor(GC_word* desc)
    {
        Object::fillGCDescriptor(desc);

        GC_set_bit(desc, GC_WORD_OFFSET(AsyncGeneratorObject, m_executionPauser.m_executionState));
        GC_set_bit(desc, GC_WORD_OFFSET(AsyncGeneratorObject, m_executionPauser.m_sourceObject));
        GC_set_bit(desc, GC_WORD_OFFSET(AsyncGeneratorObject, m_executionPauser.m_registerFile));
        GC_set_bit(desc, GC_WORD_OFFSET(AsyncGeneratorObject, m_executionPauser.m_byteCodeBlock));
        GC_set_bit(desc, GC_WORD_OFFSET(AsyncGeneratorObject, m_executionPauser.m_resumeValue));
    }

    friend Value asyncGeneratorEnqueue(ExecutionState& state, const Value& generator, AsyncGeneratorObject::AsyncGeneratorEnqueueType type, const Value& value);
    friend Value asyncGeneratorResumeNext(ExecutionState& state, AsyncGeneratorObject* generator);
    friend Value asyncGeneratorResolve(ExecutionState& state, AsyncGeneratorObject* generator, const Value& value, bool done);
    friend Value asyncGeneratorReject(ExecutionState& state, AsyncGeneratorObject* generator, Value exception);
    friend Value asyncGeneratorResumeNextReturnProcessorFulfilledFunction(ExecutionState& state, ScriptAsyncGeneratorFunctionHelperFunctionObject* F, const Value& value);
    friend Value asyncGeneratorResumeNextReturnProcessorRejectedFunction(ExecutionState& state, ScriptAsyncGeneratorFunctionHelperFunctionObject* F, const Value& reason);

    AsyncGeneratorState m_asyncGeneratorState;
    ExecutionPauser m_executionPauser;

    struct AsyncGeneratorQueueData {
        PromiseReaction::Capability m_capability; // [[Capability]]
        AsyncGeneratorEnqueueType m_operationType; // [[Completion]]
        SmallValue m_value; // [[Completion]]

        AsyncGeneratorQueueData(const PromiseReaction::Capability& capability,
                                AsyncGeneratorEnqueueType operationType,
                                const Value& value)
            : m_capability(capability)
            , m_operationType(operationType)
            , m_value(value)
        {
        }
    };
    // [[AsyncGeneratorQueue]]
    Vector<AsyncGeneratorQueueData, GCUtil::gc_malloc_allocator<AsyncGeneratorQueueData>> m_asyncGeneratorQueue;
};

// https://www.ecma-international.org/ecma-262/10.0/index.html#sec-asyncgeneratorenqueue
Value asyncGeneratorEnqueue(ExecutionState& state, const Value& generator, AsyncGeneratorObject::AsyncGeneratorEnqueueType type, const Value& value);

// https://www.ecma-international.org/ecma-262/10.0/index.html#sec-asyncgeneratorresumenext
Value asyncGeneratorResumeNext(ExecutionState& state, AsyncGeneratorObject* generator);
// https://www.ecma-international.org/ecma-262/10.0/index.html#sec-asyncgeneratorresolve
Value asyncGeneratorResolve(ExecutionState& state, AsyncGeneratorObject* generator, const Value& value, bool done);
// https://www.ecma-international.org/ecma-262/10.0/index.html#sec-asyncgeneratorreject
Value asyncGeneratorReject(ExecutionState& state, AsyncGeneratorObject* generator, Value exception);
}

#endif