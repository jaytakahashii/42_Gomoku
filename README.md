# Python ML Project Template

機械学習プロジェクト用のベーステンプレート
高品質な開発環境（Lint, Format, Type Check, CI）がセットアップ済みです。

## 📦 含まれている機能

- **Linter**: Flake8 (構文チェック)
- **Formatter**: autopep8 (Python), Prettier (その他)
- **Type Checker**: Mypy (静的型解析)
- **Settings**: VS Code 推奨設定完備
- **CI**: GitHub Actions (PR 時に静的解析を自動実行)

## 🚀 開始手順

### 1. プロジェクトの作成

1. このリポジトリ右上の **"Use this template"** ボタンから新しいリポジトリを作成
2. 作成したリポジトリをローカルにクローン

### 2. 環境構築

以下のコマンド順に実行してセットアップ

```bash
# 1. Pythonバージョンの確認 (必要に応じて .python-version を書き換えてください)
cat .python-version

# 2. 仮想環境の作成
python3 -m venv .venv

# 3. 仮想環境のアクティベート
# macOS / Linux:
source .venv/bin/activate
# Windows:
.venv\Scripts\activate

# 4. 開発用ツールのインストール
pip install -r requirements-dev.txt
```

### 3. VS Code の設定

拡張機能タブで `@recommended` と検索して一括インストールしてください。

### 4. ソースコードディレクトリの作成

このプロジェクトは Src Layout を採用しています。ルート直下にファイルを置かず、プロジェクト名のディレクトリを作成してコードを格納してください。

```bash
mkdir [project_name]
touch [project_name]/__init__.py
touch [project_name]/main.py
```

実行時は `python -m [project_name].main` の形式を推奨します。

## 🛠️ パッケージ管理ルール（運用方法）

依存関係をきれいに保つため、`pip freeze > requirements.txt` は使用禁止です。以下のルールに従ってください。

### 新しいパッケージを追加する場合

1. `pip install [package_name]` でインストール
2. 動作確認後、**トップレベルのパッケージ名のみ** を `requirements.txt` に手動で追加（依存関係で勝手に入ったサブパッケージは書きません）

| 追記先ファイル          | 用途                         | 例                     |
| ----------------------- | ---------------------------- | ---------------------- |
| `requirements.txt`      | 本番環境で必要なパッケージ   | numpy, pandas          |
| `requirements-dev.txt`  | 開発環境で必要なパッケージ   | flake8, mypy, autopep8 |
| `requirements-test.txt` | テスト環境で必要なパッケージ | pytest, pytest-cov     |
