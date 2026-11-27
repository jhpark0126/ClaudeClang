// ClaudeClang CI
// 모든 단계는 개발 환경과 같은 이미지(claudeclangbuild)에서 실행한다.
// 게이트는 ci/*.sh 에 있고, 로컬에서도 같은 스크립트를 쓴다: bash scripts/dev.sh bash ci/all.sh
//
// 한 게이트가 실패해도 나머지를 계속 실행해 한 번에 모든 부적합을 볼 수 있게 하고,
// 빌드 결과는 FAILURE 로 남긴다 (PR은 이 결과가 SUCCESS 일 때만 완료할 수 있다).

def IMAGE = 'claudeclangbuild:latest'

// 빌드 컨테이너가 Jenkins 작업 공간을 그대로 보도록 --volumes-from 으로 공유한다.
def inImage(String image, String cmd) {
    sh "docker run --rm --volumes-from \"\$JENKINS_CONTAINER_NAME\" -w \"\$WORKSPACE\" ${image} ${cmd}"
}

pipeline {
    agent any

    options {
        timestamps()
        ansiColor('xterm')
        timeout(time: 30, unit: 'MINUTES')
        buildDiscarder(logRotator(numToKeepStr: '30'))
        disableConcurrentBuilds()
    }

    stages {
        stage('빌드 이미지') {
            steps {
                // 저장소의 docker/Dockerfile 로 이미지를 만든다 (캐시되면 즉시 끝난다).
                sh "docker build -f docker/Dockerfile -t ${IMAGE} ."
                script { inImage(IMAGE, 'tool-versions') }
            }
        }
        stage('빌드') {
            steps { catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') { script { inImage(IMAGE, 'bash ci/build.sh') } } }
        }
        stage('TDD 테스트') {
            steps { catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') { script { inImage(IMAGE, 'bash ci/run_tests.sh tdd') } } }
        }
        stage('단위 테스트') {
            steps { catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') { script { inImage(IMAGE, 'bash ci/run_tests.sh unit') } } }
        }
        stage('통합 테스트') {
            steps { catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') { script { inImage(IMAGE, 'bash ci/run_tests.sh integration') } } }
        }
        stage('정적 분석 (Cppcheck)') {
            steps { catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') { script { inImage(IMAGE, 'bash ci/static_analysis.sh') } } }
        }
        stage('메트릭 (Lizard)') {
            steps { catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') { script { inImage(IMAGE, 'bash ci/metrics.sh') } } }
        }
        stage('주석률 (CLOC)') {
            steps { catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') { script { inImage(IMAGE, 'bash ci/comment_ratio.sh') } } }
        }
        stage('사람 확인용 태그') {
            steps { catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') { script { inImage(IMAGE, 'bash ci/doc_tags.sh') } } }
        }
        stage('Doxygen') {
            steps { catchError(buildResult: 'FAILURE', stageResult: 'FAILURE') { script { inImage(IMAGE, 'bash ci/doxygen.sh') } } }
        }
    }

    post {
        always {
            // 테스트 결과 (묶음별 파일은 build/reports/{tdd,unit,integration}/results.xml)
            junit allowEmptyResults: true, testResults: 'build/reports/*/results.xml'

            // 정적 분석 (Cppcheck) — 지적 1건이라도 있으면 품질 게이트 실패
            recordIssues enabledForFailure: true, skipPublishingChecks: true,
                tools: [cppCheck(pattern: 'build/reports/cppcheck.xml')],
                qualityGates: [[threshold: 1, type: 'TOTAL', criticality: 'FAILURE']]

            // 커버리지: 시험 묶음마다 따로 기록한다
            recordCoverage id: 'tdd-coverage',  name: 'TDD 커버리지',  skipPublishingChecks: true,
                tools: [[parser: 'COBERTURA', pattern: 'build/reports/tdd/cobertura.xml']]
            recordCoverage id: 'unit-coverage', name: '단위 커버리지', skipPublishingChecks: true,
                tools: [[parser: 'COBERTURA', pattern: 'build/reports/unit/cobertura.xml']]
            recordCoverage id: 'it-coverage',   name: '통합 커버리지', skipPublishingChecks: true,
                tools: [[parser: 'COBERTURA', pattern: 'build/reports/integration/cobertura.xml']]

            // MC/DC 등 Cobertura로 표현되지 않는 상세 리포트와 Lizard 결과는 HTML로 게시
            publishHTML target: [allowMissing: true, alwaysLinkToLastBuild: true, keepAll: true,
                reportDir: 'build/reports/unit/html', reportFiles: 'index.html', reportName: '단위 MC/DC 커버리지 (llvm-cov)']
            publishHTML target: [allowMissing: true, alwaysLinkToLastBuild: true, keepAll: true,
                reportDir: 'build/reports/integration/html', reportFiles: 'index.html', reportName: '통합 함수 커버리지 (llvm-cov)']
            publishHTML target: [allowMissing: true, alwaysLinkToLastBuild: true, keepAll: true,
                reportDir: 'build/reports/tdd/html', reportFiles: 'index.html', reportName: 'TDD 커버리지 (llvm-cov)']
            publishHTML target: [allowMissing: true, alwaysLinkToLastBuild: true, keepAll: true,
                reportDir: 'build/reports', reportFiles: 'lizard.html', reportName: '순환복잡도·함수 길이 (Lizard)']
            publishHTML target: [allowMissing: true, alwaysLinkToLastBuild: true, keepAll: true,
                reportDir: 'build/reports/doxygen/html', reportFiles: 'index.html', reportName: 'Doxygen 문서']

            // 검증 결과 대시보드 (VERReport): 위 결과 파일을 모아 총괄·상세를 한 묶음으로 보여 준다.
            // 판정은 각 게이트가 하므로 리포트 생성 실패가 빌드 결과를 바꾸지 않게 한다.
            sh """docker run --rm --volumes-from "\$JENKINS_CONTAINER_NAME" -w "\$WORKSPACE" \
                -e BUILD_NUMBER -e JOB_NAME -e BRANCH_NAME -e GIT_BRANCH -e GIT_COMMIT -e BUILD_URL -e WORKSPACE \
                -e VERREPORT_BUILD_TIME=${currentBuild.startTimeInMillis} \
                ${IMAGE} verreport build/reports -o build/reports/verreport || true"""
            publishHTML target: [allowMissing: true, alwaysLinkToLastBuild: true, keepAll: true,
                reportDir: 'build/reports/verreport', reportFiles: 'index.html', reportName: '검증 결과 대시보드 (VERReport)']

            archiveArtifacts artifacts: 'build/reports/**', allowEmptyArchive: true, fingerprint: true
        }
    }
}
