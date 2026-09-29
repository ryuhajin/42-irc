#!/bin/bash

SERVER="localhost"
PORT=6668
PASSWORD="0230"
NICKNAME="flood"
USERNAME="flood"
CHANNEL="#test"

RESULT_FILE="/tmp/flood_test_result.txt"
LOCK_FILE="/tmp/flood_test_result.lock"

# 초기화
echo "0 0 0" > $RESULT_FILE

trap 'cleanup' INT

cleanup() {
    echo "테스트를 종료하고 결과를 수집 중입니다..."
    pkill -P $$
    print_results
    rm -f $RESULT_FILE $LOCK_FILE
    exit 0
}

print_results() {
    read connections messages nicknames < $RESULT_FILE
    echo "========== 플러딩 테스트 결과 =========="
    echo "1. 연결 플러딩 테스트:"
    echo "   - 총 시도된 연결 수: $connections"
    echo "2. 닉네임 플러딩 테스트:"
    echo "   - 생성된 고유 닉네임 수: $nicknames"
    echo "3. 채널 입장/퇴장 플러딩 테스트:"
    echo "   - 채널 입장/퇴장 시도 횟수: $connections"
    echo "4. 메시지 플러딩 테스트:"
    echo "   - 전송된 총 메시지 수: $messages"
    echo "5. 인증 플러딩 테스트:"
    echo "   - 인증 시도 횟수: $connections"
    echo "6. 반복적인 연결/연결 해제 테스트:"
    echo "   - 연결/연결 해제 사이클 수: $connections"
    echo "========================================"
}

flood_irc() {
    local connections=0
    local messages=0
    local nicknames=0
    local messages_per_conn=50

    for ((i=0; i<10; i++)); do
        if { 
            echo "PASS ${PASSWORD}"
            echo "NICK ${NICKNAME}${RANDOM}"
            echo "USER ${USERNAME} 0 * :Flood Test"
            echo "JOIN ${CHANNEL}"
            for j in $(seq 1 $messages_per_conn); do
                echo "PRIVMSG ${CHANNEL} :Flood message $j"
            done
            echo "QUIT"
        } | nc -w 5 ${SERVER} ${PORT}; then
            ((connections++))
            ((messages+=messages_per_conn))
            ((nicknames++))
        fi
        sleep 0.1
    done
    
    (
        flock -x 200
        read total_conn total_msg total_nick < $RESULT_FILE
        echo "$((total_conn+connections)) $((total_msg+messages)) $((total_nick+nicknames))" > $RESULT_FILE
    ) 200>$LOCK_FILE
}

echo "플러딩 테스트가 시작되었습니다. Ctrl+C를 눌러 중지하거나 자동 완료를 기다리세요."

for i in {1..10}; do
    flood_irc &
done

wait

echo "모든 테스트가 완료되었습니다."
print_results
rm -f $RESULT_FILE $LOCK_FILE