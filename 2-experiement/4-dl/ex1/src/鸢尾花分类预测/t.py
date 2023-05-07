import torch
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
                p.data.add_(-lr, velocity)
                if closure is not None:
                    return closure()
    
    
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
                p.data.add_(m_hat, alpha = - (lr * m_t) / (torch.sqrt(v_hat) + eps))
                if closure is not None:
                    return closure()