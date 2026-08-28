import json
import math
import os
import subprocess
from typing import Any

import optuna

# =============================================================================
#  多点スタート焼きなまし のパラメータを optuna で自動調整するスクリプト
# =============================================================================
#  【使い方】
#    1. pip install optuna
#    2. cargo install pahcer                      (未インストールなら)
#    3. ../tools/gen.py などでテストケースを ../tools/in/ に用意する
#    4. pahcer_config.toml を対象の問題に合わせて書き換える
#    5. python3 optimize.py
#
#  【仕組み】
#    ここで作った dict が環境変数として pahcer -> a.out に渡り、
#    main.cpp の load_params() の pick_env("p1", ...) が読み取る。
#    ★ キー("p1","p2",...)を main.cpp 側と必ず一致させること。
#
#  【main.cpp のパラメータ対応】
#    p1 : TEMP_START       開始温度
#    p2 : TEMP_END         終了温度
#    p3 : NUM_STARTS       スタート回数(時間をこの数で等分)
#    p4 : P_ORO            近傍の選び方(デモ: or-opt を選ぶ確率)
# =============================================================================


# TODO: 探索するパラメータをここに書く
def generate_params(trial: optuna.trial.Trial) -> dict[str, str]:
    # 指定方法は https://optuna.readthedocs.io/en/stable/reference/generated/optuna.trial.Trial.html
    params = {
        # p1: 開始温度
        "p1": str(trial.suggest_float("p1", 1.0, 1000.0, log=True)),
        # p2: 終了温度
        "p2": str(trial.suggest_float("p2", 0.01, 10.0, log=True)),
        # p3: スタート回数
        "p3": str(trial.suggest_int("p3", 1, 16)),
        # p4: 近傍の選び方 (デモ: or-opt を選ぶ確率)
        "p4": str(trial.suggest_float("p4", 0.0, 1.0)),
    }
    return params


# TODO: スコアの取り出し方をここで調整する
def extract_score(result: dict[str, Any]) -> float:
    absolute_score = result["score"]  # noqa: F841
    log10_score = math.log10(absolute_score) if absolute_score > 0.0 else 0.0  # noqa: F841
    relative_score = result["relative_score"]  # noqa: F841

    score = absolute_score  # 絶対スコアの問題
    # score = log10_score       # 相対スコアの問題 (対数を取る場合)
    # score = relative_score    # 相対スコアの問題

    return score


# TODO: ★最小化 / 最大化 の切り替え (main.cpp の MAXIMIZE と合わせること)
def get_direction() -> str:
    direction = "minimize"     # ← スコア最小化 (デモの TSP はこちら)
    # direction = "maximize"   # ← スコア最大化
    return direction


# TODO: 試行回数 or 制限時間(秒)
def run_optimization(study: optuna.study.Study) -> None:
    # study.optimize(Objective(), timeout=1200)
    study.optimize(Objective(), n_trials=200)


class Objective:
    def __init__(self) -> None:
        pass

    def __call__(self, trial: optuna.trial.Trial) -> float:
        params = generate_params(trial)
        env = os.environ.copy()
        env.update(params)

        scores = []

        cmd = [
            "pahcer",
            "run",
            "--json",
            "--shuffle",
            "--no-result-file",
            "--freeze-best-scores",
        ]

        if trial.number != 0:
            cmd.append("--no-compile")

        process = subprocess.Popen(
            cmd,
            stdout=subprocess.PIPE,
            env=env,
        )

        # see also: https://tech.preferred.jp/ja/blog/wilcoxonpruner/
        for line in process.stdout:
            result = json.loads(line)

            # If an error occurs, stop the process and raise an exception
            if result["error_message"] != "":
                process.send_signal(subprocess.signal.SIGINT)
                raise RuntimeError(result["error_message"])

            score = extract_score(result)
            seed = result["seed"]
            scores.append(score)
            trial.report(score, seed)

            if trial.should_prune():
                print(f"Trial {trial.number} pruned.")
                process.send_signal(subprocess.signal.SIGINT)

                objective_value = sum(scores) / len(scores)
                is_better_than_best = (
                    trial.study.direction == optuna.study.StudyDirection.MINIMIZE
                    and objective_value < trial.study.best_value
                ) or (
                    trial.study.direction == optuna.study.StudyDirection.MAXIMIZE
                    and objective_value > trial.study.best_value
                )

                if is_better_than_best:
                    # Avoid updating the best value
                    raise optuna.TrialPruned()
                else:
                    # It is recommended to return the value of the objective function at the current step
                    # instead of raising TrialPruned.
                    # This is a workaround to report the evaluation information of the pruned Trial to Optuna.
                    return sum(scores) / len(scores)

        return sum(scores) / len(scores)


study = optuna.create_study(
    direction=get_direction(),
    study_name="optuna-study",
    pruner=optuna.pruners.WilcoxonPruner(),
    sampler=optuna.samplers.TPESampler(),
)

run_optimization(study)

print(f"best params = {study.best_params}")
print(f"best score  = {study.best_value}")
