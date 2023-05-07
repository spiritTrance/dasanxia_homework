import torch
import torch.nn as nn
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import torch.nn.functional as F
from torch.utils.data import TensorDataset
from torch.utils.data import DataLoader
from sklearn.model_selection import train_test_split

# CONFIG
batch = 8
epochs = 250
isMLP = False
isCNN = True
KERNAL_SIZE = 2
OUTPUT_CHANNEL = 16
data = pd.read_csv(r'iris.csv')
# 将鸢尾花种类由字母转换成数字
data['Species']=pd.factorize(data.Species)[0]
# 保存“萼片长度”，“萼片宽度”，“花瓣长度”，“花瓣宽度”
X = data.iloc[:,1:-1].values
# 保存鸢尾花种类
Y = data.Species.values
# CNN 需要(N, C, L)的数据
if isMLP == False:
    X = X.reshape(X.data.shape[0], 1, X.data.shape[1])
# 划分训练集和测试集 默认6：4
train_x,test_x,train_y,test_y=train_test_split(X,Y)
# 将numpy数据转换成tensor
train_x = torch.from_numpy(train_x).type(torch.FloatTensor)
train_y = torch.from_numpy(train_y).type(torch.LongTensor)
test_x = torch.from_numpy(test_x).type(torch.FloatTensor)
test_y = torch.from_numpy(test_y).type(torch.LongTensor)

# 创建训练集
train_ds = TensorDataset(train_x,train_y)
train_dl = DataLoader(train_ds,batch_size=batch,shuffle=True)
# 创建测试集
test_ds = TensorDataset(test_x,test_y)
test_dl = DataLoader(test_ds,batch_size=batch)

class MLPModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.model = nn.Sequential(
            nn.Linear(4, 32),
            nn.ReLU(True),
            nn.Linear(32, 32),
            nn.ReLU(True),
            nn.Linear(32, 3),
        )

    def forward(self,input):
        x = self.model(input)
        return x

class CNNModel(nn.Module):
    def __init__(self):
        super().__init__()
        self.model = nn.Sequential(
            nn.Conv1d(1, OUTPUT_CHANNEL, kernel_size=KERNAL_SIZE),
            nn.ReLU(True),
            nn.MaxPool1d(kernel_size=KERNAL_SIZE),
            nn.ReLU(),
            nn.Dropout(p=0.5),
            nn.ReLU(),
            nn.Flatten(),
            nn.LazyLinear(3),
        )

    def forward(self,input):
        x = self.model(input)
        return x
    

class SGD(torch.optim.Optimizer):
    def __init__(self, params, lr=0.01):
        defaults = dict(lr=lr)
        super(SGD, self).__init__(params, defaults)
    def step(self, closure = None):
        loss = None
        if closure is not None:
            loss = closure()
        for group in self.param_groups:
            lr = group['lr']
            for p in group['params']:
                if p.grad is None:
                    continue
                grad = p.grad.data
                p.data.add_(grad, alpha=-lr)
                return loss
class AdamOptimizer(torch.optim.Optimizer):
    def __init__(self, params, lr=0.01, betas=(0.9, 0.999), eps = 1e-8):
        defaults = dict(lr=lr, betas=betas, eps = eps)
        super(AdamOptimizer, self).__init__(params, defaults)
    def step(self, closure=None):
        for group in self.param_groups:
            for p in group['params']:
                if p.grad is None:
                    continue
                # 获取基本参数
                grad = p.grad.data
                lr = group['lr']
                eps = group['eps']
                beta1, beta2 = group["betas"]
                state = self.state[p]
                # 初始化参数
                if len(state) == 0:
                    state['m'] = torch.zeros_like(p.data)
                    state['v'] = torch.zeros_like(p.data)
                v = state['v']
                m = state['m']
                g_t = p.grad.data
                m_t = beta1 * m + (1 - beta1) * g_t
                v_t = beta2 * v + (1 - beta2) * g_t ** 2
                m_hat = m_t / (1 - beta1)
                v_hat = v_t / (1 - beta2)
                p.data.add_(m_hat / (eps + torch.sqrt(v_hat)), alpha = -lr)
                if closure is not None:
                    return closure()
class MomentumOptimizer(torch.optim.Optimizer):
    def __init__(self, params, lr=0.01, momentum=0.9):
        defaults = dict(lr=lr, momentum=momentum)
        # 调用父类构造函数
        super(MomentumOptimizer, self).__init__(params, defaults)
    def step(self, closure=None):
        for group in self.param_groups:
            for p in group['params']:
                if p.grad is None:
                    continue
                grad = p.grad.data
                lr = group['lr']
                momentum = group['momentum']
                state = self.state[p]
                if len(state) == 0:
                    state['velocity'] = torch.zeros_like(p.data)
                velocity = state['velocity']
                velocity.mul_(momentum).add_(grad)
                p.data.add_(velocity, alpha = -lr)
                if closure is not None:
                    return closure()


def accuracy(y_pred,y_true):
    # torch.argmax将数字转换成真正的预测结果
    y_pred = torch.argmax(y_pred,dim=1)
    acc = (y_pred == y_true).float().mean()
    return acc


def trainModel(model, loss_fn, optim):
    train_loss=[]
    train_acc=[]
    test_loss=[]
    test_acc=[]
    for epoch in range(epochs):
        # 训练
        for x,y in train_dl:
            model.train()
            y_pred = model(x)
            loss = loss_fn(y_pred, y)
            optim.zero_grad()
            loss.backward()
            optim.step()
        # 测试
        with torch.no_grad():
            model.eval()
            epoch_accuracy = accuracy(model(train_x),train_y)       # 训练集准确率
            epoch_loss = loss_fn(model(train_x), train_y).data      # 训练集loss
            epoch_test_accuracy = accuracy(model(test_x),test_y)    # 测试集准确率
            epoch_test_loss = loss_fn(model(test_x), test_y).data   # 测试集loss
            if epoch % 50 == 0:
                print('epoch: ',epoch,'train_loss: ',round(epoch_loss.item(),3),'train_accuracy: ',round(epoch_accuracy.item(),3),
                    'test_loss: ',round(epoch_test_loss.item(),3),'test_accuracy: ',round(epoch_test_accuracy.item(),3)
                    )
            train_loss.append(epoch_loss)
            train_acc.append(epoch_accuracy)
            test_loss.append(epoch_test_loss)
            test_acc.append(epoch_test_accuracy)
    print('final test_accuracy: ',epoch_test_accuracy)
    return train_loss, train_acc, test_loss, test_acc, float(epoch_test_accuracy)

# 画图
def drawFig(train_loss, train_acc, test_loss, test_acc, path, final_test_acc):
    plt.cla()
    plt.plot(range(1,epochs+1),train_loss,label='train_loss')
    plt.plot(range(1,epochs+1),test_loss,label='test_loss')
    plt.plot(range(1,epochs+1),train_acc,label='train_acc')
    plt.plot(range(1,epochs+1),test_acc,label='test_acc')
    plt.legend()
    plt.title(path + "  final_testacc: " + str(final_test_acc))
    plt.savefig(path)

## Config
LR = 0.0001
if isMLP:
    ## MLP + SGD_Write
    model = MLPModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = SGD(model.parameters(), lr = LR)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "MLP_SGD_Write", final_test_acc)
    ## MLP + Adam_Write
    model = MLPModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = AdamOptimizer(model.parameters(), lr = LR)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "MLP_Adam_Write", final_test_acc)
    ## MLP + Momentum_Write
    model = MLPModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = MomentumOptimizer(model.parameters(), lr = LR)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "MLP_Momentum_Write", final_test_acc)
    ## MLP + Adam
    model = MLPModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = torch.optim.Adam(model.parameters(),lr=LR)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "MLP_Adam", final_test_acc)
    ## MLP + SGD
    model = MLPModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = torch.optim.SGD(model.parameters(),lr = LR)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "MLP_SGD", final_test_acc)
    ## MLP + Momentum
    model = MLPModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = torch.optim.SGD(params=model.parameters(),lr=LR, momentum=0.9)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "MLP_Momentum", final_test_acc)
if isCNN == True:
    ## CNN + SGD_Write
    model = CNNModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = SGD(model.parameters(), lr = LR)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "CNN_SGD_Write", final_test_acc)
    ## CNN + Adam_Write
    model = CNNModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = AdamOptimizer(model.parameters(), lr = LR)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "CNN_Adam_Write", final_test_acc)
    ## CNN + Momentum_Write
    model = CNNModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = MomentumOptimizer(model.parameters(), lr = LR)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "CNN_Momentum_Write", final_test_acc)
    ## CNN + Adam
    model = CNNModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = torch.optim.Adam(model.parameters(),lr=LR)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "CNN_Adam", final_test_acc)
    ## CNN + SGD
    model = CNNModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = torch.optim.SGD(model.parameters(),lr = LR)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "CNN_SGD", final_test_acc)
    ## CNN + Momentum
    model = CNNModel()
    loss_fn = nn.CrossEntropyLoss()
    optim_adam = torch.optim.SGD(params=model.parameters(),lr=LR, momentum=0.9)
    train_loss, train_acc, test_loss, test_acc, final_test_acc= trainModel(model, loss_fn, optim_adam)
    drawFig(train_loss, train_acc, test_loss, test_acc, "CNN_Momentum", final_test_acc)