# Apache Airflow DAG 完全指南

## 1. DAG 基础概念

DAG (Directed Acyclic Graph) 是 Airflow 中的核心概念，定义了一个工作流，包含多个任务以及它们之间的依赖关系。

### 1.1 DAG 生命周期

```
DAG定义 → 调度器解析 → 生成DAG Run → 任务实例化 → 任务执行 → 完成/失败处理
```

- **调度器**：解析 DAG 文件，生成 DAG Run（运行实例）
- **执行器**：处理任务的实际执行（本地、分布式、容器化）
- **Web UI**：展示 DAG 结构图、运行状态、日志，支持手动触发

### 1.2 核心参数

```python
with DAG(
    dag_id='example_dag',               # DAG 唯一标识符
    start_date=datetime(2025, 1, 1),    # 开始日期（过去时间则看 catchup 设置）
    schedule_interval='0 12 * * *',     # 调度间隔（cron 表达式或预设值如 @daily）
    catchup=False,                      # 是否回填历史任务
    max_active_runs=1,                  # 同时运行的最大 DAG Run 数
    default_args={                      # 任务默认参数
        'owner': 'data_team',
        'retries': 3,                   # 重试次数
        'retry_delay': timedelta(minutes=5),  # 重试间隔
        'execution_timeout': timedelta(hours=1),  # 执行超时
        'email_on_failure': True,
        'email': ['team@example.com'],
    },
    tags=['example', 'tutorial'],       # 标签（Web UI 过滤用）
) as dag:
    # 任务定义...
```

## 2. 任务与操作符 (Operators)

### 2.1 常用操作符

```python
# Python 操作符
task1 = PythonOperator(
    task_id='process_data',
    python_callable=process_function,
    op_kwargs={'param1': 'value1'},
    provide_context=True,  # 在 Airflow 2.0+ 已默认为 True
)

# Bash 操作符
task2 = BashOperator(
    task_id='run_script',
    bash_command='python /path/to/script.py {{ ds }}',  # 使用 Jinja 模板
    env={'ENV_VAR': 'value'},
)

# SQL 操作符
task3 = PostgresOperator(
    task_id='query_db',
    postgres_conn_id='postgres_default',
    sql="SELECT * FROM table WHERE date = '{{ ds }}'",
)

# Sensor 操作符 - 等待条件满足
task4 = FileSensor(
    task_id='wait_for_file',
    filepath='/data/{{ ds }}/input.csv',
    poke_interval=60,  # 每60秒检查一次
    timeout=60 * 60,   # 最多等待1小时
)

# 分支操作符 - 条件流程控制
task5 = BranchPythonOperator(
    task_id='check_condition',
    python_callable=decide_which_path,
)
```

### 2.2 任务依赖关系

```python
# 方法1: 位移操作符
task1 >> task2 >> task3

# 方法2: 位移操作符 - 多对多关系
task1 >> [task2, task3] >> task4

# 方法3: set_upstream/set_downstream 方法
task4.set_upstream(task3)
task2.set_downstream(task3)

# 方法4: 并行执行
[task2, task3] >> task4  # task2和task3都完成后执行task4
```

## 3. 任务间通信 (XCom)

XCom (Cross-Communication) 是 Airflow 中任务间传递数据的机制。

### 3.1 基本用法

```python
# 推送数据
def push_data(**context):
    # 方法1: 自动推送返回值
    return {'key': 'value'}
    
    # 方法2: 手动推送 - 可推送多个值
    context['ti'].xcom_push(key='result', value={'status': 'success'})
    context['ti'].xcom_push(key='count', value=42)

# 拉取数据
def pull_data(**context):
    # 方法1: 通过 context 拉取
    result = context['ti'].xcom_pull(task_ids='push_task', key='result')
    
    # 方法2: 拉取任务的返回值
    data = context['ti'].xcom_pull(task_ids='push_task')
    
    print(f"Retrieved data: {result}, {data}")

push_task = PythonOperator(
    task_id='push_task',
    python_callable=push_data,
)

pull_task = PythonOperator(
    task_id='pull_task',
    python_callable=pull_data,
)

push_task >> pull_task
```

### 3.2 在模板中使用 XCom

```python
bash_task = BashOperator(
    task_id='use_xcom',
    bash_command='echo "Value from previous task: {{ ti.xcom_pull(task_ids="push_task", key="result") }}"',
)
```

### 3.3 XCom 最佳实践

- **限制数据大小**：XCom 存储在元数据数据库中，不适合大量数据传输
- **数据类型**：确保数据可序列化 (JSON 兼容)
- **数据安全**：敏感信息应使用 Variables 或 Connections 管理
- **替代方案**：大型数据应使用文件系统、数据库等外部存储

## 4. 工程化实践模式

### 4.1 DAG 工厂模式

将 DAG 定义封装为工厂函数，提高复用性。

```python
def create_data_processing_dag(
    dag_id,
    schedule_interval,
    source_table,
    target_table,
    start_date=datetime(2025, 1, 1),
):
    """创建数据处理 DAG 的工厂函数"""
    
    default_args = {
        'owner': 'data_team',
        'retries': 2,
        'retry_delay': timedelta(minutes=5),
    }
    
    with DAG(
        dag_id=dag_id,
        schedule_interval=schedule_interval,
        start_date=start_date,
        catchup=False,
        default_args=default_args,
    ) as dag:
        
        extract = PythonOperator(
            task_id='extract_data',
            python_callable=extract_data,
            op_kwargs={'table': source_table},
        )
        
        transform = PythonOperator(
            task_id='transform_data',
            python_callable=transform_data,
        )
        
        load = PythonOperator(
            task_id='load_data',
            python_callable=load_data,
            op_kwargs={'table': target_table},
        )
        
        extract >> transform >> load
        
        return dag

# 实例化多个相似结构的 DAG
dag1 = create_data_processing_dag(
    dag_id='process_sales',
    schedule_interval='@daily',
    source_table='raw_sales',
    target_table='processed_sales',
)

dag2 = create_data_processing_dag(
    dag_id='process_users',
    schedule_interval='@weekly',
    source_table='raw_users',
    target_table='processed_users',
)
```

### 4.2 配置驱动的 DAG 生成

结合 YAML/JSON 配置文件动态生成 DAG。

```python
# dag_factory.py
def create_dag_from_config(config):
    with DAG(
        dag_id=config['dag_id'],
        schedule_interval=config['schedule_interval'],
        start_date=datetime.strptime(config['start_date'], '%Y-%m-%d'),
        catchup=config.get('catchup', False),
    ) as dag:
        
        # 创建任务...
        for task_config in config['tasks']:
            if task_config['type'] == 'python':
                # 动态导入函数
                module_path, function_name = task_config['callable'].rsplit('.', 1)
                module = importlib.import_module(module_path)
                func = getattr(module, function_name)
                
                task = PythonOperator(
                    task_id=task_config['task_id'],
                    python_callable=func,
                    op_kwargs=task_config.get('kwargs', {}),
                )
            
            # 支持其他操作符类型...
        
        # 设置依赖关系
        for dep in config['dependencies']:
            upstream_id, downstream_id = dep
            dag.get_task(upstream_id) >> dag.get_task(downstream_id)
            
        return dag

# 从配置文件加载
import yaml
import glob
import os

dags_folder = os.path.join(os.path.dirname(__file__), 'dag_configs')
for config_file in glob.glob(f"{dags_folder}/*.yaml"):
    with open(config_file) as f:
        config = yaml.safe_load(f)
        dag_id = config['dag_id']
        globals()[dag_id] = create_dag_from_config(config)
```

### 4.3 面向对象的 DAG 设计

通过类封装 DAG 定义，利用继承和组合提高复用性。

```python
class BaseDataPipeline:
    """基础数据管道抽象类"""
    
    def __init__(
        self,
        dag_id,
        schedule_interval,
        start_date=datetime(2025, 1, 1),
        default_args=None,
    ):
        self.dag_id = dag_id
        self.schedule_interval = schedule_interval
        self.start_date = start_date
        self.default_args = default_args or {
            'owner': 'data_team',
            'retries': 3,
            'retry_delay': timedelta(minutes=5),
        }
        
        self.dag = self.create_dag()
    
    def create_dag(self):
        """创建基础 DAG"""
        with DAG(
            dag_id=self.dag_id,
            schedule_interval=self.schedule_interval,
            start_date=self.start_date,
            default_args=self.default_args,
            catchup=False,
        ) as dag:
            self.define_tasks(dag)
            return dag
    
    def define_tasks(self, dag):
        """定义任务和依赖关系，子类需要实现这个方法"""
        raise NotImplementedError("Subclasses must implement this method")

class ETLPipeline(BaseDataPipeline):
    """提取-转换-加载数据管道"""
    
    def __init__(self, dag_id, schedule_interval, source, destination, **kwargs):
        self.source = source
        self.destination = destination
        super().__init__(dag_id, schedule_interval, **kwargs)
    
    def define_tasks(self, dag):
        extract = PythonOperator(
            task_id='extract',
            python_callable=self.extract_data,
            dag=dag,
        )
        
        transform = PythonOperator(
            task_id='transform',
            python_callable=self.transform_data,
            dag=dag,
        )
        
        load = PythonOperator(
            task_id='load',
            python_callable=self.load_data,
            dag=dag,
        )
        
        extract >> transform >> load
    
    def extract_data(self, **context):
        print(f"Extracting data from {self.source}")
        return {"raw_data": f"data_from_{self.source}"}
    
    def transform_data(self, **context):
        ti = context['ti']
        extracted = ti.xcom_pull(task_ids='extract')
        print(f"Transforming data: {extracted}")
        return {"transformed_data": f"transformed_{extracted['raw_data']}"}
    
    def load_data(self, **context):
        ti = context['ti']
        transformed = ti.xcom_pull(task_ids='transform')
        print(f"Loading data to {self.destination}: {transformed}")

# 实例化数据管道
sales_pipeline = ETLPipeline(
    dag_id='sales_etl',
    schedule_interval='@daily',
    source='sales_db',
    destination='data_warehouse',
)

# 获取 DAG 对象供 Airflow 调度器使用
sales_dag = sales_pipeline.dag
```

### 4.4 任务组合模式

将常用任务组合封装为可复用的函数。

```python
def create_data_quality_tasks(data_source, dag=None):
    """创建一组数据质量检查任务"""
    
    check_nulls = PythonOperator(
        task_id=f'check_nulls_{data_source}',
        python_callable=check_null_values,
        op_kwargs={'source': data_source},
        dag=dag,
    )
    
    check_duplicates = PythonOperator(
        task_id=f'check_duplicates_{data_source}',
        python_callable=check_duplicate_records,
        op_kwargs={'source': data_source},
        dag=dag,
    )
    
    validation_complete = DummyOperator(
        task_id=f'validation_complete_{data_source}',
        dag=dag,
    )
    
    [check_nulls, check_duplicates] >> validation_complete
    
    return validation_complete  # 返回终结任务方便建立依赖

with DAG('data_pipeline', ...) as dag:
    # 数据提取任务
    extract = PythonOperator(...)
    
    # 添加质量检查任务组
    validation = create_data_quality_tasks('customer_data', dag)
    
    # 后续处理任务
    process = PythonOperator(...)
    
    # 设置依赖
    extract >> validation >> process
```

### 4.5 条件执行模式

使用分支操作符实现条件流程控制。

```python
def decide_processing_path(**context):
    """根据条件决定执行路径"""
    dag_run = context['dag_run']
    
    # 从 DAG 运行配置获取参数
    data_size = dag_run.conf.get('data_size', 0)
    
    if data_size > 1000000:
        return 'process_large_dataset'
    else:
        return 'process_small_dataset'

branch_task = BranchPythonOperator(
    task_id='branch_task',
    python_callable=decide_processing_path,
)

large_process = PythonOperator(
    task_id='process_large_dataset',
    python_callable=process_large_data,
)

small_process = PythonOperator(
    task_id='process_small_dataset',
    python_callable=process_small_data,
)

join = DummyOperator(
    task_id='join',
    trigger_rule='one_success',  # 只要有一个上游任务成功即触发
)

branch_task >> [large_process, small_process] >> join
```

### 4.6 任务组 (TaskGroups)

使用任务组结构化组织复杂 DAG。

```python
from airflow.utils.task_group import TaskGroup

with DAG('complex_pipeline', ...) as dag:
    start = DummyOperator(task_id='start')
    
    # 提取任务组
    with TaskGroup(group_id='extract') as extract_group:
        extract_task1 = PythonOperator(task_id='extract_customers')
        extract_task2 = PythonOperator(task_id='extract_products')
        extract_task3 = PythonOperator(task_id='extract_sales')
    
    # 转换任务组
    with TaskGroup(group_id='transform') as transform_group:
        transform_task1 = PythonOperator(task_id='transform_customers')
        transform_task2 = PythonOperator(task_id='transform_products')
        transform_task3 = PythonOperator(task_id='transform_sales')
        
        # 设置组内依赖
        [extract_task1, extract_task2, extract_task3] >> transform_task1
        transform_task1 >> [transform_task2, transform_task3]
    
    end = DummyOperator(task_id='end')
    
    # 设置组间依赖
    start >> extract_group >> transform_group >> end
```

## 5. 高级案例：数据监控与报告系统

一个完整的示例，结合上述多种设计模式。

```python
"""
数据监控与报告系统
- 定期检查数据质量
- 生成报告
- 条件性触发警报
"""

from datetime import datetime, timedelta
from airflow import DAG
from airflow.operators.python import PythonOperator, BranchPythonOperator
from airflow.operators.bash import BashOperator
from airflow.operators.dummy import DummyOperator
from airflow.utils.task_group import TaskGroup
from airflow.models import Variable
import json
import pandas as pd
import smtplib
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

# 定义默认参数
default_args = {
    'owner': 'data_team',
    'depends_on_past': False,
    'email_on_failure': True,
    'email': ['data-alerts@example.com'],
    'retries': 1,
    'retry_delay': timedelta(minutes=5),
}

# 工具函数
def check_data_quality(table_name, **context):
    """检查数据质量"""
    # 模拟数据质量检查
    print(f"Checking data quality for {table_name}")
    
    # 生成随机质量指标
    import random
    metrics = {
        'completeness': random.uniform(0.9, 1.0),
        'accuracy': random.uniform(0.85, 1.0),
        'timeliness': random.uniform(0.8, 1.0)
    }
    
    # 推送指标到XCom
    context['ti'].xcom_push(key=f'quality_metrics_{table_name}', value=metrics)
    return metrics

def evaluate_metrics(**context):
    """评估指标并决定是否发送警报"""
    tables = context['params']['tables']
    all_metrics = {}
    
    # 收集所有表的指标
    ti = context['ti']
    for table in tables:
        metrics = ti.xcom_pull(key=f'quality_metrics_{table}')
        all_metrics[table] = metrics
    
    # 检查是否有任何指标低于阈值
    has_issues = False
    for table, metrics in all_metrics.items():
        for metric, value in metrics.items():
            if value < 0.9:  # 阈值
                has_issues = True
                print(f"Issue detected: {table}.{metric} = {value}")
    
    # 推送完整报告
    context['ti'].xcom_push(key='quality_report', value=all_metrics)
    
    # 根据结果决定分支
    if has_issues:
        return 'send_alert'
    else:
        return 'skip_alert'

def generate_report(**context):
    """生成质量报告"""
    ti = context['ti']
    report_data = ti.xcom_pull(key='quality_report')
    
    # 创建报告
    report = "数据质量报告\n"
    report += "=" * 40 + "\n"
    report += f"生成时间: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}\n\n"
    
    for table, metrics in report_data.items():
        report += f"表: {table}\n"
        for metric, value in metrics.items():
            status = "✅ 通过" if value >= 0.9 else "❌ 失败"
            report += f"  - {metric}: {value:.2f} {status}\n"
        report += "\n"
    
    # 保存报告到文件
    report_path = f"/tmp/quality_report_{context['ds_nodash']}.txt"
    with open(report_path, "w") as f:
        f.write(report)
    
    return report_path

def send_email_alert(**context):
    """发送警报邮件"""
    ti = context['ti']
    report_data = ti.xcom_pull(key='quality_report')
    report_path = ti.xcom_pull(task_ids='generate_report')
    
    # 读取报告内容
    with open(report_path, "r") as f:
        report_content = f.read()
    
    print(f"发送警报邮件，内容: {report_content[:100]}...")
    # 实际邮件发送代码...
    
    return True

# 创建 DAG 工厂函数
def create_monitoring_dag(
    dag_id,
    schedule_interval,
    tables,
    email_recipients=None,
    start_date=datetime(2025, 1, 1),
):
    """创建数据监控 DAG"""
    
    with DAG(
        dag_id=dag_id,
        default_args=default_args,
        schedule_interval=schedule_interval,
        start_date=start_date,
        catchup=False,
        tags=['monitoring', 'data-quality'],
    ) as dag:
        
        start = DummyOperator(task_id='start')
        
        # 为每个表创建质量检查任务组
        with TaskGroup(group_id='quality_checks') as quality_group:
            check_tasks = []
            for table in tables:
                task = PythonOperator(
                    task_id=f'check_{table}',
                    python_callable=check_data_quality,
                    op_kwargs={'table_name': table},
                )
                check_tasks.append(task)
        
        # 评估结果
        evaluate = BranchPythonOperator(
            task_id='evaluate_metrics',
            python_callable=evaluate_metrics,
            params={'tables': tables},
        )
        
        # 生成报告
        report = PythonOperator(
            task_id='generate_report',
            python_callable=generate_report,
        )
        
        # 发送警报
        send_alert = PythonOperator(
            task_id='send_alert',
            python_callable=send_email_alert,
        )
        
        # 跳过警报
        skip_alert = DummyOperator(task_id='skip_alert')
        
        # 终结任务
        end = DummyOperator(
            task_id='end',
            trigger_rule='one_success',  # 只要有一个上游任务成功即触发
        )
        
        # 设置依赖关系
        start >> quality_group >> evaluate
        evaluate >> [send_alert, skip_alert]
        evaluate >> report
        [report, send_alert, skip_alert] >> end
        
        return dag

# 实例化监控 DAG
sales_monitoring = create_monitoring_dag(
    dag_id='sales_data_monitoring',
    schedule_interval='0 9 * * *',  # 每天上午9点
    tables=['sales', 'customers', 'products'],
    email_recipients=['team@example.com'],
)
```

## 6. 总结与最佳实践

### 设计模式摘要

1. **工厂模式**：通过函数生成 DAG，提高复用性
2. **配置驱动**：使用配置文件控制 DAG 行为
3. **面向对象**：通过类封装 DAG 定义，利用继承和组合
4. **任务组合**：将常用任务组合封装为可复用函数
5. **条件执行**：使用分支操作符实现流程控制
6. **任务组**：结构化组织复杂 DAG

### 性能与维护性建议

1. **保持 DAG 文件简洁**：每个文件不超过一个 DAG
2. **合理命名**：使用清晰一致的命名约定
3. **参数化配置**：使用 Variables 和 Connections 管理配置
4. **避免无谓依赖**：只在必要时建立任务依赖
5. **合理分组**：逻辑相关的任务应放在一起
6. **监控重要指标**：运行时间、失败率、资源使用
7. **文档和注释**：记录 DAG 的目的、输入和输出
8. **版本控制**：使用 Git 管理 DAG 代码
9. **测试 DAG**：编写单元测试和集成测试

