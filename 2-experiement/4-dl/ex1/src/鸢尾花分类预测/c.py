
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