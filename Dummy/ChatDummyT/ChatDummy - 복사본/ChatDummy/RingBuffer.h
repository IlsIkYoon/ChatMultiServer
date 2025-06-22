#pragma once

class RingBuffer
{
public:
    RingBuffer();
    RingBuffer(int size);
    ~RingBuffer();

    void Resize();
    int GetBufferSize() const;
    int GetUseSize() const;
    int GetFreeSize() const;

    int Enqueue(const char* data, int size);
    int Dequeue(char* data, int size);
    int Peek(char* data, int size) const;

    int DirectEnqueueSize() const;
    int DirectDequeueSize() const;

    char* GetFrontBuffer() { return buffer + readPos; }
    char* GetRearBuffer() { return buffer + writePos; }

    void ClearBuffer();

    int MoveFront(int size);
    int MoveRear(int size);

    //
    char* GetRingBuffer() { return buffer; }

private:
    char* buffer;       // 버퍼 배열
    int bufferSize;     // 버퍼 전체 크기
    int readPos;        // 읽기 위치
    int writePos;       // 쓰기 위치
    int resizeCnt;

    bool IsFull() const;
    bool IsEmpty() const;
};
